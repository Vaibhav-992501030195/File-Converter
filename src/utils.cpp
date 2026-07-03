#include "utils.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ufc {
namespace utils {

std::string toLowerExtension(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

std::string extractFilename(const std::string& path) {
    std::filesystem::path p(path);
    return p.stem().string();
}

std::string extractExtension(const std::string& path) {
    std::filesystem::path p(path);
    std::string ext = p.extension().string();
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1);
    }
    return toLowerExtension(ext);
}

bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

std::string getOutputPath(const std::string& outputFolder, const std::string& inputFile,
                          const std::string& newExt) {
    std::string filename = extractFilename(inputFile);
    std::filesystem::path outDir(outputFolder);
    std::filesystem::path outFile = outDir / (filename + "." + newExt);
    return outFile.string();
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void pauseScreen() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void printSeparator(char ch, int width) {
    std::cout << std::string(width, ch) << std::endl;
}

}
}
