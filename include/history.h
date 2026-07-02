#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

namespace ufc {

struct HistoryEntry {
    std::string timestamp;
    std::string filename;
    std::string conversion;
    std::string status;
};

class History {
public:
    explicit History(const std::string& historyDir);
    void record(const std::string& filename, const std::string& fromFormat,
                const std::string& toFormat, bool success);
    void display() const;

private:
    std::string historyFile;
    void ensureDirectoryExists() const;
};

}

#endif
