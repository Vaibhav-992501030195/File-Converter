#include "data_converter.h"
#include "validator.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <algorithm>

namespace ufc {

std::string DataConverter::name() const {
    return "Data Converter";
}

std::vector<std::pair<FileType, FileType>> DataConverter::supportedFormats() const {
    return {
        {FileType::CSV, FileType::JSON},
        {FileType::JSON, FileType::CSV},
        {FileType::JSON, FileType::XML},
        {FileType::XML, FileType::JSON},
        {FileType::CSV, FileType::XML},
        {FileType::XML, FileType::CSV}
    };
}

std::string DataConverter::validate(const std::string& inputPath, FileType outputType) {
    FileType inputType = Detector::detect(inputPath);
    return Validator::validate(inputPath, inputType, outputType);
}

namespace {

struct CsvData {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

std::string trimWhitespace(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return str.substr(start, end - start + 1);
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(trimWhitespace(field));
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(trimWhitespace(field));
    return fields;
}

CsvData parseCsv(const std::string& filePath) {
    CsvData data;
    std::ifstream file(filePath);
    std::string line;

    if (std::getline(file, line)) {
        data.headers = splitCsvLine(line);
    }

    while (std::getline(file, line)) {
        if (trimWhitespace(line).empty()) continue;
        data.rows.push_back(splitCsvLine(line));
    }

    return data;
}

std::string escapeJsonString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

std::string escapeXml(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&apos;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

void csvToJson(const std::string& inputPath, const std::string& outputPath) {
    CsvData data = parseCsv(inputPath);
    std::ofstream out(outputPath);

    out << "[\n";
    for (size_t i = 0; i < data.rows.size(); i++) {
        out << "  {\n";
        for (size_t j = 0; j < data.headers.size() && j < data.rows[i].size(); j++) {
            out << "    \"" << escapeJsonString(data.headers[j]) << "\": \""
                << escapeJsonString(data.rows[i][j]) << "\"";
            if (j < data.headers.size() - 1 && j < data.rows[i].size() - 1) {
                out << ",";
            }
            out << "\n";
        }
        out << "  }";
        if (i < data.rows.size() - 1) out << ",";
        out << "\n";
    }
    out << "]\n";
}

void csvToXml(const std::string& inputPath, const std::string& outputPath) {
    CsvData data = parseCsv(inputPath);
    std::ofstream out(outputPath);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<data>\n";
    for (const auto& row : data.rows) {
        out << "  <record>\n";
        for (size_t j = 0; j < data.headers.size() && j < row.size(); j++) {
            std::string tag = data.headers[j];
            std::replace(tag.begin(), tag.end(), ' ', '_');
            out << "    <" << tag << ">" << escapeXml(row[j]) << "</" << tag << ">\n";
        }
        out << "  </record>\n";
    }
    out << "</data>\n";
}

void jsonTocsv(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream file(inputPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::vector<std::string> keys;
    std::vector<std::vector<std::string>> rows;

    size_t pos = 0;
    bool firstObject = true;

    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

        size_t objEnd = content.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = content.substr(objStart + 1, objEnd - objStart - 1);
        std::vector<std::string> values;

        size_t kvPos = 0;
        while (kvPos < obj.size()) {
            size_t keyStart = obj.find('"', kvPos);
            if (keyStart == std::string::npos) break;

            size_t keyEnd = obj.find('"', keyStart + 1);
            if (keyEnd == std::string::npos) break;

            std::string key = obj.substr(keyStart + 1, keyEnd - keyStart - 1);

            size_t valStart = obj.find('"', keyEnd + 1);
            std::string value;

            if (valStart != std::string::npos && valStart < obj.find(',', keyEnd + 1)) {
                size_t valEnd = obj.find('"', valStart + 1);
                if (valEnd != std::string::npos) {
                    value = obj.substr(valStart + 1, valEnd - valStart - 1);
                    kvPos = valEnd + 1;
                } else {
                    kvPos = obj.size();
                }
            } else {
                size_t colonPos = obj.find(':', keyEnd + 1);
                if (colonPos != std::string::npos) {
                    size_t valEndComma = obj.find(',', colonPos + 1);
                    if (valEndComma == std::string::npos) valEndComma = obj.size();
                    value = trimWhitespace(obj.substr(colonPos + 1, valEndComma - colonPos - 1));
                    kvPos = valEndComma + 1;
                } else {
                    kvPos = obj.size();
                }
            }

            if (firstObject) {
                keys.push_back(key);
            }
            values.push_back(value);
        }

        rows.push_back(values);
        firstObject = false;
        pos = objEnd + 1;
    }

    std::ofstream out(outputPath);
    for (size_t i = 0; i < keys.size(); i++) {
        out << keys[i];
        if (i < keys.size() - 1) out << ",";
    }
    out << "\n";

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); i++) {
            out << row[i];
            if (i < row.size() - 1) out << ",";
        }
        out << "\n";
    }
}

void jsonToXml(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream file(inputPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::ofstream out(outputPath);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<data>\n";

    size_t pos = 0;
    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

        size_t objEnd = content.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = content.substr(objStart + 1, objEnd - objStart - 1);
        out << "  <record>\n";

        size_t kvPos = 0;
        while (kvPos < obj.size()) {
            size_t keyStart = obj.find('"', kvPos);
            if (keyStart == std::string::npos) break;

            size_t keyEnd = obj.find('"', keyStart + 1);
            if (keyEnd == std::string::npos) break;

            std::string key = obj.substr(keyStart + 1, keyEnd - keyStart - 1);
            std::replace(key.begin(), key.end(), ' ', '_');

            size_t valStart = obj.find('"', keyEnd + 1);
            std::string value;

            if (valStart != std::string::npos && valStart < obj.find(',', keyEnd + 1)) {
                size_t valEnd = obj.find('"', valStart + 1);
                if (valEnd != std::string::npos) {
                    value = obj.substr(valStart + 1, valEnd - valStart - 1);
                    kvPos = valEnd + 1;
                } else {
                    kvPos = obj.size();
                }
            } else {
                size_t colonPos = obj.find(':', keyEnd + 1);
                if (colonPos != std::string::npos) {
                    size_t valEndComma = obj.find(',', colonPos + 1);
                    if (valEndComma == std::string::npos) valEndComma = obj.size();
                    value = trimWhitespace(obj.substr(colonPos + 1, valEndComma - colonPos - 1));
                    kvPos = valEndComma + 1;
                } else {
                    kvPos = obj.size();
                }
            }

            out << "    <" << key << ">" << escapeXml(value) << "</" << key << ">\n";
        }

        out << "  </record>\n";
        pos = objEnd + 1;
    }

    out << "</data>\n";
}

void xmlToCsv(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream file(inputPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;

    size_t pos = 0;
    bool firstRecord = true;

    while (pos < content.size()) {
        size_t recStart = content.find("<record>", pos);
        if (recStart == std::string::npos) break;

        size_t recEnd = content.find("</record>", recStart);
        if (recEnd == std::string::npos) break;

        std::string record = content.substr(recStart + 8, recEnd - recStart - 8);
        std::vector<std::string> values;

        size_t tagPos = 0;
        while (tagPos < record.size()) {
            size_t openStart = record.find('<', tagPos);
            if (openStart == std::string::npos) break;

            size_t openEnd = record.find('>', openStart);
            if (openEnd == std::string::npos) break;

            std::string tag = record.substr(openStart + 1, openEnd - openStart - 1);
            if (tag[0] == '/' || tag[0] == '?') {
                tagPos = openEnd + 1;
                continue;
            }

            std::string closeTag = "</" + tag + ">";
            size_t closePos = record.find(closeTag, openEnd);
            if (closePos == std::string::npos) {
                tagPos = openEnd + 1;
                continue;
            }

            std::string value = record.substr(openEnd + 1, closePos - openEnd - 1);

            if (firstRecord) {
                headers.push_back(tag);
            }
            values.push_back(value);
            tagPos = closePos + closeTag.size();
        }

        rows.push_back(values);
        firstRecord = false;
        pos = recEnd + 9;
    }

    std::ofstream out(outputPath);
    for (size_t i = 0; i < headers.size(); i++) {
        out << headers[i];
        if (i < headers.size() - 1) out << ",";
    }
    out << "\n";

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); i++) {
            out << row[i];
            if (i < row.size() - 1) out << ",";
        }
        out << "\n";
    }
}

void xmlToJson(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream file(inputPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::ofstream out(outputPath);
    out << "[\n";

    size_t pos = 0;
    bool firstRecord = true;

    while (pos < content.size()) {
        size_t recStart = content.find("<record>", pos);
        if (recStart == std::string::npos) break;

        size_t recEnd = content.find("</record>", recStart);
        if (recEnd == std::string::npos) break;

        if (!firstRecord) out << ",\n";
        firstRecord = false;

        std::string record = content.substr(recStart + 8, recEnd - recStart - 8);
        out << "  {\n";

        size_t tagPos = 0;
        bool firstField = true;
        while (tagPos < record.size()) {
            size_t openStart = record.find('<', tagPos);
            if (openStart == std::string::npos) break;

            size_t openEnd = record.find('>', openStart);
            if (openEnd == std::string::npos) break;

            std::string tag = record.substr(openStart + 1, openEnd - openStart - 1);
            if (tag[0] == '/' || tag[0] == '?') {
                tagPos = openEnd + 1;
                continue;
            }

            std::string closeTag = "</" + tag + ">";
            size_t closePos = record.find(closeTag, openEnd);
            if (closePos == std::string::npos) {
                tagPos = openEnd + 1;
                continue;
            }

            std::string value = record.substr(openEnd + 1, closePos - openEnd - 1);

            if (!firstField) out << ",\n";
            firstField = false;

            out << "    \"" << escapeJsonString(tag) << "\": \"" << escapeJsonString(value) << "\"";
            tagPos = closePos + closeTag.size();
        }

        out << "\n  }";
        pos = recEnd + 9;
    }

    out << "\n]\n";
}

}

bool DataConverter::convert(const std::string& inputPath, const std::string& outputPath,
                            FileType outputType) {
    try {
        std::filesystem::path outDir = std::filesystem::path(outputPath).parent_path();
        if (!std::filesystem::exists(outDir)) {
            std::filesystem::create_directories(outDir);
        }

        FileType inputType = Detector::detect(inputPath);

        if (inputType == FileType::CSV && outputType == FileType::JSON) {
            csvToJson(inputPath, outputPath);
        }
        else if (inputType == FileType::CSV && outputType == FileType::XML) {
            csvToXml(inputPath, outputPath);
        }
        else if (inputType == FileType::JSON && outputType == FileType::CSV) {
            jsonTocsv(inputPath, outputPath);
        }
        else if (inputType == FileType::JSON && outputType == FileType::XML) {
            jsonToXml(inputPath, outputPath);
        }
        else if (inputType == FileType::XML && outputType == FileType::CSV) {
            xmlToCsv(inputPath, outputPath);
        }
        else if (inputType == FileType::XML && outputType == FileType::JSON) {
            xmlToJson(inputPath, outputPath);
        }
        else {
            throw std::runtime_error("Unsupported data conversion.");
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Conversion error: " << e.what() << std::endl;
        return false;
    }
}

}
