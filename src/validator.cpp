#include "validator.h"
#include "utils.h"
#include <fstream>
#include <set>
#include <utility>

namespace ufc {

bool Validator::fileExists(const std::string& path) {
    return utils::fileExists(path);
}

bool Validator::isReadable(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

bool Validator::isSupportedConversion(FileType input, FileType output) {
    static const std::set<std::pair<FileType, FileType>> supported = {
        {FileType::PNG, FileType::JPG}, {FileType::JPG, FileType::PNG},
        {FileType::BMP, FileType::PNG}, {FileType::PNG, FileType::BMP},

        {FileType::MP3, FileType::WAV}, {FileType::WAV, FileType::MP3},

        {FileType::MP4, FileType::AVI}, {FileType::AVI, FileType::MP4},
        {FileType::MKV, FileType::MP4}, {FileType::MP4, FileType::MKV},

        {FileType::DOCX, FileType::PDF}, {FileType::PDF, FileType::TXT},

        {FileType::CSV, FileType::JSON}, {FileType::JSON, FileType::CSV},
        {FileType::JSON, FileType::XML}, {FileType::XML, FileType::JSON},
        {FileType::CSV, FileType::XML}, {FileType::XML, FileType::CSV}
    };

    return supported.count({input, output}) > 0;
}

bool Validator::formatsAreDifferent(FileType input, FileType output) {
    return input != output;
}

std::string Validator::validate(const std::string& inputPath, FileType inputType, FileType outputType) {
    if (!fileExists(inputPath)) {
        return "Error: File '" + inputPath + "' does not exist.";
    }

    if (!isReadable(inputPath)) {
        return "Error: File '" + inputPath + "' is not readable. Check permissions.";
    }

    if (inputType == FileType::UNKNOWN) {
        return "Error: Input file format is not recognized.";
    }

    if (outputType == FileType::UNKNOWN) {
        return "Error: Output format is not recognized.";
    }

    if (!formatsAreDifferent(inputType, outputType)) {
        return "Error: Input and output formats are the same.";
    }

    if (!isSupportedConversion(inputType, outputType)) {
        return "Error: Conversion from " + Detector::fileTypeToString(inputType)
               + " to " + Detector::fileTypeToString(outputType) + " is not supported.";
    }

    return "";
}

}
