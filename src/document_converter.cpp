#include "document_converter.h"
#include "validator.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <iomanip>

namespace ufc {

std::string DocumentConverter::name() const {
    return "Document Converter";
}

std::vector<std::pair<FileType, FileType>> DocumentConverter::supportedFormats() const {
    return {
        {FileType::DOCX, FileType::PDF},
        {FileType::PDF, FileType::TXT}
    };
}

std::string DocumentConverter::validate(const std::string& inputPath, FileType outputType) {
    FileType inputType = Detector::detect(inputPath);
    return Validator::validate(inputPath, inputType, outputType);
}

namespace {

std::string runCommand(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }

    std::string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

std::string extractTextFromDocx(const std::string& docxPath) {
    std::string cmd = "unzip -p \"" + docxPath + "\" word/document.xml 2>/dev/null";
    std::string xml = runCommand(cmd);

    if (xml.empty()) {
        throw std::runtime_error("Could not read DOCX content. File may be corrupted.");
    }

    std::string text;
    size_t pos = 0;

    while (pos < xml.size()) {
        size_t paraEnd = xml.find("</w:p>", pos);
        if (paraEnd == std::string::npos) {
            paraEnd = xml.size();
        }

        std::string paragraphText;
        size_t searchPos = pos;

        while (searchPos < paraEnd) {
            size_t tStart = xml.find("<w:t", searchPos);
            if (tStart == std::string::npos || tStart >= paraEnd) break;

            size_t contentStart = xml.find('>', tStart);
            if (contentStart == std::string::npos) break;
            contentStart++;

            size_t contentEnd = xml.find("</w:t>", contentStart);
            if (contentEnd == std::string::npos || contentEnd > paraEnd) break;

            paragraphText += xml.substr(contentStart, contentEnd - contentStart);
            searchPos = contentEnd + 6;
        }

        if (!paragraphText.empty()) {
            text += paragraphText + "\n";
        } else if (paraEnd < xml.size()) {
            text += "\n";
        }

        if (paraEnd >= xml.size()) break;
        pos = paraEnd + 6;
    }

    return text;
}

std::string escapePdfString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        if (c == '(' || c == ')' || c == '\\') {
            escaped += '\\';
        }
        if (c >= 32 && c <= 126) {
            escaped += c;
        } else if (c == '\t') {
            escaped += "    ";
        }
    }
    return escaped;
}

std::vector<std::string> wrapText(const std::string& text, int maxCharsPerLine) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string paragraph;

    while (std::getline(stream, paragraph)) {
        if (paragraph.empty()) {
            lines.push_back("");
            continue;
        }

        while (!paragraph.empty() && (paragraph.back() == '\r' || paragraph.back() == '\n')) {
            paragraph.pop_back();
        }

        size_t pos = 0;
        while (pos < paragraph.size()) {
            if (paragraph.size() - pos <= static_cast<size_t>(maxCharsPerLine)) {
                lines.push_back(paragraph.substr(pos));
                break;
            }

            size_t end = pos + maxCharsPerLine;
            size_t lastSpace = paragraph.rfind(' ', end);
            if (lastSpace != std::string::npos && lastSpace > pos) {
                lines.push_back(paragraph.substr(pos, lastSpace - pos));
                pos = lastSpace + 1;
            } else {
                lines.push_back(paragraph.substr(pos, maxCharsPerLine));
                pos = end;
            }
        }
    }

    return lines;
}

void generatePdf(const std::string& outputPath, const std::string& text) {
    std::vector<std::string> allLines = wrapText(text, 85);

    int linesPerPage = 52;
    int fontSize = 11;
    int lineHeight = 14;
    int marginLeft = 56;
    int pageTopY = 730;
    int pageWidth = 612;
    int pageHeight = 792;

    int totalPages = std::max(1, static_cast<int>((allLines.size() + linesPerPage - 1) / linesPerPage));

    std::vector<std::string> contentStreams;
    for (int page = 0; page < totalPages; page++) {
        std::ostringstream cs;
        cs << "BT\n";
        cs << "/F1 " << fontSize << " Tf\n";
        cs << lineHeight << " TL\n";
        cs << marginLeft << " " << pageTopY << " Td\n";

        int startLine = page * linesPerPage;
        int endLine = std::min(startLine + linesPerPage, static_cast<int>(allLines.size()));

        for (int i = startLine; i < endLine; i++) {
            cs << "(" << escapePdfString(allLines[i]) << ") Tj T*\n";
        }

        cs << "ET\n";
        contentStreams.push_back(cs.str());
    }

    int fontObjNum = 3 + totalPages * 2;
    int totalObjects = fontObjNum;

    std::ostringstream pdf;
    std::vector<size_t> offsets;

    pdf << "%PDF-1.4\n";
    pdf << "%\xe2\xe3\xcf\xd3\n";

    offsets.push_back(static_cast<size_t>(pdf.tellp()));
    pdf << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

    offsets.push_back(static_cast<size_t>(pdf.tellp()));
    pdf << "2 0 obj\n<< /Type /Pages /Kids [";
    for (int i = 0; i < totalPages; i++) {
        if (i > 0) pdf << " ";
        pdf << (3 + i * 2) << " 0 R";
    }
    pdf << "] /Count " << totalPages << " >>\nendobj\n";

    for (int page = 0; page < totalPages; page++) {
        int pageObjNum = 3 + page * 2;
        int contentObjNum = 4 + page * 2;

        offsets.push_back(static_cast<size_t>(pdf.tellp()));
        pdf << pageObjNum << " 0 obj\n";
        pdf << "<< /Type /Page /Parent 2 0 R ";
        pdf << "/MediaBox [0 0 " << pageWidth << " " << pageHeight << "] ";
        pdf << "/Contents " << contentObjNum << " 0 R ";
        pdf << "/Resources << /Font << /F1 " << fontObjNum << " 0 R >> >> ";
        pdf << ">>\nendobj\n";

        offsets.push_back(static_cast<size_t>(pdf.tellp()));
        pdf << contentObjNum << " 0 obj\n";
        pdf << "<< /Length " << contentStreams[page].size() << " >>\n";
        pdf << "stream\n";
        pdf << contentStreams[page];
        pdf << "endstream\nendobj\n";
    }

    offsets.push_back(static_cast<size_t>(pdf.tellp()));
    pdf << fontObjNum << " 0 obj\n";
    pdf << "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>\n";
    pdf << "endobj\n";

    size_t xrefPos = static_cast<size_t>(pdf.tellp());
    pdf << "xref\n";
    pdf << "0 " << (totalObjects + 1) << "\n";
    pdf << "0000000000 65535 f \n";
    for (size_t offset : offsets) {
        pdf << std::setw(10) << std::setfill('0') << offset << " 00000 n \n";
    }

    pdf << "trailer\n";
    pdf << "<< /Size " << (totalObjects + 1) << " /Root 1 0 R >>\n";
    pdf << "startxref\n";
    pdf << xrefPos << "\n";
    pdf << "%%EOF\n";

    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot create output file: " + outputPath);
    }
    out << pdf.str();
    out.close();
}

std::string extractTextFromPdf(const std::string& pdfPath) {
    std::ifstream file(pdfPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open PDF file: " + pdfPath);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::string extracted;
    size_t pos = 0;

    while (pos < content.size()) {
        size_t streamStart = content.find("stream\n", pos);
        if (streamStart == std::string::npos) {
            streamStart = content.find("stream\r\n", pos);
            if (streamStart == std::string::npos) break;
            streamStart += 8;
        } else {
            streamStart += 7;
        }

        size_t streamEnd = content.find("endstream", streamStart);
        if (streamEnd == std::string::npos) break;

        std::string stream = content.substr(streamStart, streamEnd - streamStart);

        size_t btPos = stream.find("BT");
        size_t etPos = stream.find("ET");
        if (btPos != std::string::npos && etPos != std::string::npos) {
            std::string textBlock = stream.substr(btPos, etPos - btPos);

            size_t tPos = 0;
            while (tPos < textBlock.size()) {
                size_t openParen = textBlock.find('(', tPos);
                if (openParen == std::string::npos) break;

                int depth = 1;
                size_t closeParen = openParen + 1;
                while (closeParen < textBlock.size() && depth > 0) {
                    if (textBlock[closeParen] == '\\') {
                        closeParen++;
                    } else if (textBlock[closeParen] == '(') {
                        depth++;
                    } else if (textBlock[closeParen] == ')') {
                        depth--;
                    }
                    closeParen++;
                }

                std::string textStr = textBlock.substr(openParen + 1, closeParen - openParen - 2);

                std::string unescaped;
                for (size_t i = 0; i < textStr.size(); i++) {
                    if (textStr[i] == '\\' && i + 1 < textStr.size()) {
                        char next = textStr[i + 1];
                        if (next == 'n') { unescaped += '\n'; i++; }
                        else if (next == 'r') { unescaped += '\r'; i++; }
                        else if (next == 't') { unescaped += '\t'; i++; }
                        else if (next == '(' || next == ')' || next == '\\') { unescaped += next; i++; }
                        else { unescaped += next; i++; }
                    } else {
                        unescaped += textStr[i];
                    }
                }

                extracted += unescaped;

                size_t afterParen = closeParen;
                while (afterParen < textBlock.size() && textBlock[afterParen] == ' ') afterParen++;
                if (afterParen < textBlock.size()) {
                    std::string op;
                    while (afterParen < textBlock.size() && textBlock[afterParen] != '\n' &&
                           textBlock[afterParen] != '\r' && textBlock[afterParen] != '(') {
                        op += textBlock[afterParen];
                        afterParen++;
                    }
                    if (op.find("Tj") != std::string::npos && op.find("T*") != std::string::npos) {
                        extracted += "\n";
                    } else if (op.find("'") != std::string::npos || op.find("T*") != std::string::npos) {
                        extracted += "\n";
                    }
                }

                tPos = closeParen;
            }
        }

        pos = streamEnd + 9;
    }

    if (extracted.empty()) {
        for (size_t i = 0; i < content.size(); i++) {
            char c = content[i];
            if (c >= 32 && c <= 126) {
                extracted += c;
            } else if (c == '\n' || c == '\r') {
                if (!extracted.empty() && extracted.back() != '\n') {
                    extracted += '\n';
                }
            }
        }
    }

    return extracted;
}

}

bool DocumentConverter::convert(const std::string& inputPath, const std::string& outputPath,
                                FileType outputType) {
    try {
        std::filesystem::path outDir = std::filesystem::path(outputPath).parent_path();
        if (!std::filesystem::exists(outDir)) {
            std::filesystem::create_directories(outDir);
        }

        FileType inputType = Detector::detect(inputPath);

        if (inputType == FileType::DOCX && outputType == FileType::PDF) {
            std::string text = extractTextFromDocx(inputPath);
            if (text.empty()) {
                throw std::runtime_error("No text content found in the DOCX file.");
            }
            generatePdf(outputPath, text);
        }
        else if (inputType == FileType::PDF && outputType == FileType::TXT) {
            std::string text = extractTextFromPdf(inputPath);

            std::ofstream out(outputPath);
            if (!out.is_open()) {
                throw std::runtime_error("Cannot create output file: " + outputPath);
            }
            out << text;
            out.close();
        }
        else {
            throw std::runtime_error("Unsupported document conversion.");
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Conversion error: " << e.what() << std::endl;
        return false;
    }
}

}
