#include "detector.h"
#include "utils.h"
#include <unordered_map>

namespace ufc {

FileType Detector::detect(const std::string& filePath) {
    std::string ext = utils::extractExtension(filePath);
    return stringToFileType(ext);
}

std::string Detector::fileTypeToString(FileType type) {
    static const std::unordered_map<FileType, std::string> typeNames = {
        {FileType::PNG, "PNG"}, {FileType::JPG, "JPG"}, {FileType::BMP, "BMP"},
        {FileType::MP3, "MP3"}, {FileType::WAV, "WAV"},
        {FileType::MP4, "MP4"}, {FileType::AVI, "AVI"}, {FileType::MKV, "MKV"},
        {FileType::DOCX, "DOCX"}, {FileType::PDF, "PDF"}, {FileType::TXT, "TXT"},
        {FileType::CSV, "CSV"}, {FileType::JSON, "JSON"}, {FileType::XML, "XML"},
        {FileType::UNKNOWN, "UNKNOWN"}
    };

    auto it = typeNames.find(type);
    if (it != typeNames.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

FileType Detector::stringToFileType(const std::string& ext) {
    std::string lower = utils::toLowerExtension(ext);

    static const std::unordered_map<std::string, FileType> extMap = {
        {"png", FileType::PNG}, {"jpg", FileType::JPG}, {"jpeg", FileType::JPG},
        {"bmp", FileType::BMP},
        {"mp3", FileType::MP3}, {"wav", FileType::WAV},
        {"mp4", FileType::MP4}, {"avi", FileType::AVI}, {"mkv", FileType::MKV},
        {"docx", FileType::DOCX}, {"pdf", FileType::PDF}, {"txt", FileType::TXT},
        {"csv", FileType::CSV}, {"json", FileType::JSON}, {"xml", FileType::XML}
    };

    auto it = extMap.find(lower);
    if (it != extMap.end()) {
        return it->second;
    }
    return FileType::UNKNOWN;
}

}
