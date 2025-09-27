#pragma once
#include <string>
#include <filesystem>
#include <chrono>
#include "Constants.h"

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
        bool isExecutable;
        bool isSymlink;
        bool isHidden;
    };

    static FileInfo getFileInfo(const fs::path& path);
    static std::string formatSize(uint64_t size);
    static std::string formatSizeWithBytes(uint64_t size);
    static std::string formatSizeBothSystems(uint64_t size);
    static std::string formatNumber(uint64_t number);
    static std::string formatTime(const fs::file_time_type& time);
    static std::string formatPermissions(const fs::perms& permissions);
    static bool isHidden(const fs::path& path);
    static bool isExecutable(const fs::path& path);
    static bool isSymlink(const fs::path& path);
    static uint64_t calculateDirectorySize(const fs::path& path);
    static std::string getFileColor(const FileInfo& info);
};