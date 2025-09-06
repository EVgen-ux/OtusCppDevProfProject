#include "FileSystem.h"
#include "constants.h"
#include <iostream>

FileSystem::FileInfo FileSystem::getFileInfo(const fs::path& path) {
    FileInfo info;
    
    try {
        info.name = path.filename().string();
        info.isDirectory = fs::is_directory(path);
        
        if (!info.isDirectory) {
            info.size = fs::file_size(path);
            info.sizeFormatted = formatSize(info.size);
        } else {
            info.size = 0;
            info.sizeFormatted = "<DIR>";
        }
        
        auto ftime = fs::last_write_time(path);
        info.lastModified = formatTime(ftime);
        info.permissions = formatPermissions(fs::status(path).permissions());
    }
    catch (const fs::filesystem_error& e) {
        info.name = path.filename().string() + " [Ошибка доступа]";
        info.size = 0;
        info.sizeFormatted = "N/A";
        info.lastModified = "N/A";
        info.permissions = "N/A";
        info.isDirectory = false;
    }
    
    return info;
}

std::string FileSystem::formatSize(uint64_t size) {
    if (size >= constants::TB) {
        return std::to_string(size / constants::TB) + " TB";
    } else if (size >= constants::GB) {
        return std::to_string(size / constants::GB) + " GB";
    } else if (size >= constants::MB) {
        return std::to_string(size / constants::MB) + " MB";
    } else if (size >= constants::KB) {
        return std::to_string(size / constants::KB) + " KB";
    } else {
        return std::to_string(size) + " B";
    }
}

std::string FileSystem::formatTime(const fs::file_time_type& time) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm tm = *std::localtime(&tt);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string FileSystem::formatPermissions(const fs::perms& permissions) {
    
    std::string result;    
    result += ((permissions & fs::perms::owner_read) != fs::perms::none) ? "r" : "-";
    result += ((permissions & fs::perms::owner_write) != fs::perms::none) ? "w" : "-";
    result += ((permissions & fs::perms::owner_exec) != fs::perms::none) ? "x" : "-";
    result += ((permissions & fs::perms::group_read) != fs::perms::none) ? "r" : "-";
    result += ((permissions & fs::perms::group_write) != fs::perms::none) ? "w" : "-";
    result += ((permissions & fs::perms::group_exec) != fs::perms::none) ? "x" : "-";
    result += ((permissions & fs::perms::others_read) != fs::perms::none) ? "r" : "-";
    result += ((permissions & fs::perms::others_write) != fs::perms::none) ? "w" : "-";
    result += ((permissions & fs::perms::others_exec) != fs::perms::none) ? "x" : "-";
    
    return result;
}

bool FileSystem::isHidden(const fs::path& path) {
    std::string filename = path.filename().string();
    return !filename.empty() && filename[0] == '.';
}