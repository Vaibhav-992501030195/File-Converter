#ifndef DOCUMENT_CONVERTER_H
#define DOCUMENT_CONVERTER_H

#include "converter.h"

namespace ufc {

class DocumentConverter : public Converter {
public:
    std::string validate(const std::string& inputPath, FileType outputType) override;
    bool convert(const std::string& inputPath, const std::string& outputPath, FileType outputType) override;
    std::vector<std::pair<FileType, FileType>> supportedFormats() const override;
    std::string name() const override;
};

}

#endif
