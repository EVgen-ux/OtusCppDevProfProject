#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileSystem {
public:
    struct FileInfo {
        std::string name;
        uint64_t size;
        std::string sizeFormatted;
        std::string lastModified;
        std::string permissions;
        bool isDirectory;
    };

    static FileInfo getFileInfo(const fs::path& path);
    static std::string formatSize(uint64_t size);
    static std::string formatTime(const fs::file_time_type& time);
    static std::string formatPermissions(const fs::perms& permissions);
    static bool isHidden(const fs::path& path);
};
