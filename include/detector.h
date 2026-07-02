#ifndef DETECTOR_H
#define DETECTOR_H

#include <string>

namespace ufc {

enum class FileType {
    PNG, JPG, BMP,
    MP3, WAV,
    MP4, AVI, MKV,
    DOCX, PDF, TXT,
    CSV, JSON, XML,
    UNKNOWN
};

class Detector {
public:
    static FileType detect(const std::string& filePath);
    static std::string fileTypeToString(FileType type);
    static FileType stringToFileType(const std::string& ext);
};

}

#endif
