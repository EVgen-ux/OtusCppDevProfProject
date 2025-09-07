#include "FileSystem.h"
#include "constants.h"
#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <sstream>

namespace fs = std::filesystem;

FileSystem::FileInfo FileSystem::getFileInfo(const fs::path& path) {
    FileInfo info;
    info.name = path.filename().string();
    info.isDirectory = fs::is_directory(path);
    
    try {
        if (info.isDirectory) {
            info.size = calculateDirectorySize(path);
            info.sizeFormatted = formatSize(info.size);
        } else {
            info.size = fs::file_size(path);
            info.sizeFormatted = formatSize(info.size);
        }
        
        info.lastModified = formatTime(fs::last_write_time(path));
        info.permissions = formatPermissions(fs::status(path).permissions());
    } catch (const fs::filesystem_error& e) {
        info.size = 0;
        info.sizeFormatted = "0 B";
        info.lastModified = "N/A";
        info.permissions = "---------";
    }
    
    return info;
}

uint64_t FileSystem::calculateDirectorySize(const fs::path& path) {
    uint64_t totalSize = 0;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                try {
                    totalSize += entry.file_size();
                } catch (const fs::filesystem_error&) {
                    // Пропускаем файлы без доступа
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        // Пропускаем директории без доступа
    }
    
    return totalSize;
}

std::string FileSystem::formatSize(uint64_t size) {
    if (size < constants::KB) {
        return std::to_string(size) + " B";
    } else if (size < constants::MB) {
        return std::to_string(size / constants::KB) + " KB";
    } else if (size < constants::GB) {
        return std::to_string(size / constants::MB) + " MB";
    } else if (size < constants::TB) {
        return std::to_string(size / constants::GB) + " GB";
    } else {
        return std::to_string(size / constants::TB) + " TB";
    }
}

std::string FileSystem::formatNumber(uint64_t number) {
    std::string numStr = std::to_string(number);
    std::string result;
    
    int count = 0;
    for (int i = numStr.length() - 1; i >= 0; i--) {
        result = numStr[i] + result;
        count++;
        if (count % 3 == 0 && i != 0) {
            result = " " + result; // Используем тонкий пробел
        }
    }
    
    return result;
}

std::string FileSystem::formatSizeWithBytes(uint64_t size) {
    std::string formattedSize = formatSize(size);
    std::string formattedBytes = formatNumber(size) + " bytes";
    return formattedSize + " (" + formattedBytes + ")";
}

std::string FileSystem::formatSizeBothSystems(uint64_t size) {
    std::stringstream result;
    
    // Двоичные приставки (1 KiB = 1024 B, 1 MiB = 1024 KiB, и т.д.)
    std::stringstream ssBinary;
    ssBinary << std::fixed << std::setprecision(1);
    
    if (size < constants::KB) {
        ssBinary << size << " B";
    } else if (size < constants::MB) {
        double kbSize = static_cast<double>(size) / constants::KB;
        ssBinary << kbSize << " KiB";
    } else if (size < constants::GB) {
        double mbSize = static_cast<double>(size) / constants::MB;
        ssBinary << mbSize << " MiB";
    } else if (size < constants::TB) {
        double gbSize = static_cast<double>(size) / constants::GB;
        ssBinary << gbSize << " GiB";
    } else {
        double tbSize = static_cast<double>(size) / constants::TB;
        ssBinary << tbSize << " TiB";
    }
    
    std::string binaryResult = ssBinary.str();
    size_t dotPos = binaryResult.find('.');
    if (dotPos != std::string::npos) {
        binaryResult[dotPos] = ',';
    }
    
    // Десятичные приставки (1 KB = 1000 B, 1 MB = 1000 KB, и т.д.)
    std::stringstream ssDecimal;
    ssDecimal << std::fixed << std::setprecision(1);
    
    const uint64_t DECIMAL_KB = 1000;
    const uint64_t DECIMAL_MB = 1000 * DECIMAL_KB;
    const uint64_t DECIMAL_GB = 1000 * DECIMAL_MB;
    const uint64_t DECIMAL_TB = 1000 * DECIMAL_GB;
    
    if (size < DECIMAL_KB) {
        ssDecimal << size << " B";
    } else if (size < DECIMAL_MB) {
        double kbSize = static_cast<double>(size) / DECIMAL_KB;
        ssDecimal << kbSize << " KB";
    } else if (size < DECIMAL_GB) {
        double mbSize = static_cast<double>(size) / DECIMAL_MB;
        ssDecimal << mbSize << " MB";
    } else if (size < DECIMAL_TB) {
        double gbSize = static_cast<double>(size) / DECIMAL_GB;
        ssDecimal << gbSize << " GB";
    } else {
        double tbSize = static_cast<double>(size) / DECIMAL_TB;
        ssDecimal << tbSize << " TB";
    }
    
    std::string decimalResult = ssDecimal.str();
    dotPos = decimalResult.find('.');
    if (dotPos != std::string::npos) {
        decimalResult[dotPos] = ',';
    }
    
    // Форматируем байты с разделителями
    std::string formattedBytes = formatNumber(size) + " bytes";
    
    // Собираем все вместе
    result << binaryResult << " / " << decimalResult << " (" << formattedBytes << ")";
    
    return result.str();
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
    std::string result(9, '-');
    
    auto setPermission = [&](int pos, fs::perms perm, char ch) {
        if ((permissions & perm) != fs::perms::none) {
            result[pos] = ch;
        }
    };
    
    setPermission(0, fs::perms::owner_read, 'r');
    setPermission(1, fs::perms::owner_write, 'w');
    setPermission(2, fs::perms::owner_exec, 'x');
    setPermission(3, fs::perms::group_read, 'r');
    setPermission(4, fs::perms::group_write, 'w');
    setPermission(5, fs::perms::group_exec, 'x');
    setPermission(6, fs::perms::others_read, 'r');
    setPermission(7, fs::perms::others_write, 'w');
    setPermission(8, fs::perms::others_exec, 'x');
    
    return result;
}

bool FileSystem::isHidden(const fs::path& path) {
    std::string filename = path.filename().string();
    if (filename.empty()) return false;
    
    // Для Windows и Unix-like систем
    #ifdef _WIN32
        return filename[0] == '.';
    #else
        return filename[0] == '.';
    #endif
}