#include "converter.h"
#include "detector.h"
#include <iostream>

namespace ufc {

void Converter::displaySupportedFormats() const {
    auto formats = supportedFormats();
    std::cout << "\n  Supported conversions:" << std::endl;
    for (const auto& pair : formats) {
        std::cout << "    " << Detector::fileTypeToString(pair.first)
                  << " -> " << Detector::fileTypeToString(pair.second) << std::endl;
    }
}

}
