#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <filesystem>

namespace ufc {
namespace utils {

std::string toLowerExtension(const std::string& ext);
std::string extractFilename(const std::string& path);
std::string extractExtension(const std::string& path);
bool fileExists(const std::string& path);
std::string getCurrentTimestamp();
std::string getOutputPath(const std::string& outputFolder, const std::string& inputFile, const std::string& newExt);
void clearScreen();
void pauseScreen();
void printSeparator(char ch = '=', int width = 50);

}
}

#endif
