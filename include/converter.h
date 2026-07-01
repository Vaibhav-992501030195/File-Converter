#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <vector>
#include <utility>
#include "detector.h"

namespace ufc {

class Converter {
public:
    virtual ~Converter() = default;
    virtual std::string validate(const std::string& inputPath, FileType outputType) = 0;
    virtual bool convert(const std::string& inputPath, const std::string& outputPath, FileType outputType) = 0;
    virtual std::vector<std::pair<FileType, FileType>> supportedFormats() const = 0;
    virtual std::string name() const = 0;
    void displaySupportedFormats() const;
};

}

#endif
