#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include "detector.h"

namespace ufc {

class Validator {
public:
    static bool fileExists(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isSupportedConversion(FileType input, FileType output);
    static bool formatsAreDifferent(FileType input, FileType output);
    static std::string validate(const std::string& inputPath, FileType inputType, FileType outputType);
};

}

#endif
