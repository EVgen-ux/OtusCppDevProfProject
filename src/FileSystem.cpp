#include "FileSystem.h"
#include "ColorManager.h"
#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

FileSystem::FileInfo FileSystem::getFileInfo(const fs::path& path) {
    FileInfo info;
    info.name = path.filename().string();
    info.isDirectory = fs::is_directory(path);
    info.isHidden = isHidden(path);
    info.isExecutable = isExecutable(path);
    info.isSymlink = isSymlink(path);
    
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
            result = " " + result;
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
    
    // Двоичные приставки
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
    } else {
        double gbSize = static_cast<double>(size) / constants::GB;
        ssBinary << gbSize << " GiB";
    }
    
    std::string binaryResult = ssBinary.str();
    
    // Десятичные приставки
    const uint64_t DECIMAL_KB = 1000;
    const uint64_t DECIMAL_MB = 1000 * DECIMAL_KB;
    const uint64_t DECIMAL_GB = 1000 * DECIMAL_MB;
    
    std::stringstream ssDecimal;
    ssDecimal << std::fixed << std::setprecision(1);
    
    if (size < DECIMAL_KB) {
        ssDecimal << size << " B";
    } else if (size < DECIMAL_MB) {
        double kbSize = static_cast<double>(size) / DECIMAL_KB;
        ssDecimal << kbSize << " KB";
    } else if (size < DECIMAL_GB) {
        double mbSize = static_cast<double>(size) / DECIMAL_MB;
        ssDecimal << mbSize << " MB";
    } else {
        double gbSize = static_cast<double>(size) / DECIMAL_GB;
        ssDecimal << gbSize << " GB";
    }
    
    std::string decimalResult = ssDecimal.str();
    std::string formattedBytes = formatNumber(size) + " bytes";
    
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
    return filename[0] == '.';
}

bool FileSystem::isExecutable(const fs::path& path) {
    try {
        auto perms = fs::status(path).permissions();
        return (perms & (fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec)) != fs::perms::none;
    } catch (...) {
    }
    return false;
}

bool FileSystem::isSymlink(const fs::path& path) {
    try {
        return fs::is_symlink(path);
    } catch (...) {
    }
    return false;
}

std::string FileSystem::getFileColor(const FileInfo& info) {
    if (!ColorManager::areColorsEnabled()) {
        return "";
    }
    
    if (info.isHidden) {
        return constants::BLACK + constants::BOLD;
    }
    
    if (info.isDirectory) {
        return constants::DIR_NAME_COLOR;
    }
    
    if (info.isSymlink) {
        return constants::SYMLINK_COLOR;
    }
    
    if (info.isExecutable) {
        return constants::EXECUTABLE_COLOR;
    }
    
     std::string extension = info.name.substr(info.name.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Изображения
    if (extension == "jpg" || extension == "jpeg" || extension == "png" || 
        extension == "gif" || extension == "bmp" || extension == "svg" ||
        extension == "webp" || extension == "tiff") {
        return constants::IMAGE_COLOR;
    }
    
    // Видео файлы
    if (extension == "mov" || extension == "mp4" || extension == "avi" || 
        extension == "mkv" || extension == "wmv" || extension == "flv" ||
        extension == "webm" || extension == "m4v" || extension == "mpeg") {
        return constants::MAGENTA + constants::BOLD;  // Яркий пурпурный для видео
    }
    
    // Аудио файлы
    if (extension == "mp3" || extension == "wav" || extension == "flac" || 
        extension == "aac" || extension == "ogg" || extension == "wma") {
        return constants::CYAN + constants::BOLD;  // Яркий голубой для аудио
    }
    
    // Архивы
    if (extension == "zip" || extension == "rar" || extension == "tar" || 
        extension == "gz" || extension == "7z" || extension == "bz2" ||
        extension == "xz" || extension == "lz" || extension == "arj") {
        return constants::ARCHIVE_COLOR;
    }
    
    // Конфигурационные файлы
    if (extension == "conf" || extension == "config" || extension == "ini" || 
        extension == "json" || extension == "xml" || extension == "yaml" || 
        extension == "yml" || extension == "toml" || extension == "properties") {
        return constants::CONFIG_COLOR;
    }
    
    // Документы
    if (extension == "txt" || extension == "doc" || extension == "docx" || 
        extension == "pdf" || extension == "rtf" || extension == "odt" ||
        extension == "xls" || extension == "xlsx" || extension == "ppt" || 
        extension == "pptx" || extension == "epub" || extension == "mobi") {
        return constants::DOCUMENT_COLOR;
    }
    
    // Исходный код
    if (extension == "cpp" || extension == "h" || extension == "hpp" || 
        extension == "c" || extension == "java" || extension == "py" || 
        extension == "js" || extension == "html" || extension == "css" || 
        extension == "php" || extension == "rb" || extension == "go" ||
        extension == "rs" || extension == "swift" || extension == "kt" ||
        extension == "ts" || extension == "scala" || extension == "pl" ||
        extension == "lua" || extension == "sh" || extension == "bat" ||
        extension == "ps1" || extension == "md" || extension == "tex") {
        return constants::CODE_COLOR;
    }
    
    // Базы данных и данные
    if (extension == "db" || extension == "sql" || extension == "sqlite" || 
        extension == "mdb" || extension == "csv" || extension == "tsv" ||
        extension == "dat" || extension == "log") {
        return constants::YELLOW;  // Желтый для данных
    }
    
    // Файлы бэкапов
    if (extension == "bak" || extension == "backup" || extension == "old" ||
        extension == "tmp" || extension == "temp") {
        return constants::BLACK;  // Черный для временных файлов
    }
    
    // Шрифты
    if (extension == "ttf" || extension == "otf" || extension == "woff" || 
        extension == "woff2" || extension == "eot") {
        return constants::MAGENTA;  // Пурпурный для шрифтов
    }
    
    return constants::WHITE; // Стандартный цвет для остальных файлов
}