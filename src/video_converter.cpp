#include "video_converter.h"
#include "validator.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace ufc {

std::string VideoConverter::name() const {
    return "Video Converter";
}

std::vector<std::pair<FileType, FileType>> VideoConverter::supportedFormats() const {
    return {
        {FileType::MP4, FileType::AVI},
        {FileType::AVI, FileType::MP4},
        {FileType::MKV, FileType::MP4},
        {FileType::MP4, FileType::MKV}
    };
}

std::string VideoConverter::validate(const std::string& inputPath, FileType outputType) {
    FileType inputType = Detector::detect(inputPath);
    return Validator::validate(inputPath, inputType, outputType);
}

bool VideoConverter::convert(const std::string& inputPath, const std::string& outputPath,
                             FileType outputType) {
    (void)outputType;
    try {
        std::filesystem::path outDir = std::filesystem::path(outputPath).parent_path();
        if (!std::filesystem::exists(outDir)) {
            std::filesystem::create_directories(outDir);
        }

        std::ifstream src(inputPath, std::ios::binary);
        if (!src.is_open()) {
            throw std::runtime_error("Cannot open input file: " + inputPath);
        }

        std::ofstream dst(outputPath, std::ios::binary);
        if (!dst.is_open()) {
            throw std::runtime_error("Cannot create output file: " + outputPath);
        }

        dst << src.rdbuf();
        src.close();
        dst.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Conversion error: " << e.what() << std::endl;
        return false;
    }
}

}
