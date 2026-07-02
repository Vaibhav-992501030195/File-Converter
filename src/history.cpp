#include "history.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace ufc {

History::History(const std::string& historyDir) {
    std::filesystem::path dir(historyDir);
    historyFile = (dir / "history.txt").string();
    ensureDirectoryExists();
}

void History::ensureDirectoryExists() const {
    std::filesystem::path dir = std::filesystem::path(historyFile).parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

void History::record(const std::string& filename, const std::string& fromFormat,
                     const std::string& toFormat, bool success) {
    std::ofstream file(historyFile, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not write to history file." << std::endl;
        return;
    }

    file << utils::getCurrentTimestamp() << "\n";
    file << filename << "\n";
    file << fromFormat << " -> " << toFormat << "\n";
    file << (success ? "Success" : "Failed") << "\n";
    file << "\n";
    file.close();
}

void History::display() const {
    std::ifstream file(historyFile);
    if (!file.is_open() || std::filesystem::file_size(historyFile) == 0) {
        std::cout << "No conversion history found." << std::endl;
        return;
    }

    std::cout << "\n";
    utils::printSeparator('=', 50);
    std::cout << "        Conversion History" << std::endl;
    utils::printSeparator('=', 50);

    std::string line;
    int entryCount = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            std::cout << std::string(50, '-') << std::endl;
            entryCount++;
            continue;
        }
        std::cout << line << std::endl;
    }
    file.close();

    std::cout << "\nTotal conversions: " << entryCount << std::endl;
}

}
