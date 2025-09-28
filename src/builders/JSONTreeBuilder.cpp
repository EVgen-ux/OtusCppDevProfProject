#include "JSONTreeBuilder.h"
#include "ColorManager.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

JSONTreeBuilder::JSONTreeBuilder(const std::string& rootPath) 
    : TreeBuilder(rootPath) {}

void JSONTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    hiddenObjectsCount_ = 0;
    
    // Строим JSON структуру
    jsonData_ = traverseDirectoryJSON(rootPath_, showHidden, true);
    
    // Добавляем статистику в корень JSON
    jsonData_["statistics"] = {
        {"directories", stats_.totalDirectories},
        {"files", stats_.totalFiles},
        {"totalSize", stats_.totalSize},
        {"totalSizeFormatted", FileSystem::formatSize(stats_.totalSize)}
    };
    
    // Для обратной совместимости создаем текстовое представление
    treeLines_.push_back("JSON output available - use getJSON() method");
}

void JSONTreeBuilder::printTree() const {
    std::cout << jsonData_.dump(2) << std::endl;
}

std::string JSONTreeBuilder::getJSON() const {
    return jsonData_.dump(2);
}

json JSONTreeBuilder::traverseDirectoryJSON(const fs::path& path, 
                                          bool showHidden,
                                          bool isRoot) {
    json node;
    
    if (isRoot) {
        node["path"] = path.string();
        node["name"] = "";
        node["type"] = "directory";
    } else {
        auto info = FileSystem::getFileInfo(path);
        node = fileInfoToJSON(info);
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
    }
    
    std::vector<fs::directory_entry> entries;
    json contents = json::array();
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            bool isHidden = FileSystem::isHidden(entry.path());
            
            if (!isHidden || showHidden) {
                entries.push_back(entry);
            } else {
                hiddenObjectsCount_++;
            }
        }
    } catch (const fs::filesystem_error&) {
        node["error"] = "Permission denied";
        return node;
    }
    
    // Сортировка: сначала директории, потом файлы
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.path().filename().string() < b.path().filename().string();
    });
    
    for (const auto& entry : entries) {
        if (entry.is_directory()) {
            // РЕКУРСИВНЫЙ ОБХОД для директорий
            json childNode = traverseDirectoryJSON(entry.path(), showHidden, false);
            contents.push_back(childNode);
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            contents.push_back(fileInfoToJSON(info));
            
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
    
    if (!contents.empty()) {
        node["contents"] = contents;
    }
    
    return node;
}

json JSONTreeBuilder::fileInfoToJSON(const FileSystem::FileInfo& info) {
    return {
        {"name", info.name},
        {"type", info.isDirectory ? "directory" : "file"},
        {"size", info.size},
        {"sizeFormatted", info.sizeFormatted},
        {"lastModified", info.lastModified},
        {"permissions", info.permissions},
        {"isHidden", info.isHidden},
        {"isExecutable", info.isExecutable},
        {"isSymlink", info.isSymlink}
    };
}