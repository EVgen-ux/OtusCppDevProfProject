#include "TreeBuilder.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

TreeBuilder::TreeBuilder(const std::string& rootPath) : rootPath_(rootPath) {}

void TreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    hiddenObjectsCount_ = 0;

    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    traverseDirectory(rootPath_, "", true, showHidden, true);
}

void TreeBuilder::traverseDirectory(const fs::path& path, 
                                  const std::string& prefix, 
                                  bool isLast,
                                  bool showHidden,
                                  bool isRoot) {
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        
        treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
    }
    
    std::string newPrefix = prefix;
    if (isLast) {
        newPrefix += constants::TREE_SPACE;
    } else {
        newPrefix += constants::TREE_VERTICAL;
    }
    
    std::vector<fs::directory_entry> entries;
    
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
        return;
    }
    
    // Сортировка: сначала директории, потом файлы
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.path().filename().string() < b.path().filename().string();
    });
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        
        if (entry.is_directory()) {
            traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
            
            treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
}

std::string TreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                      const std::string& connector) const {
    std::string nameColor = FileSystem::getFileColor(info);  
    if (info.isDirectory) {
        return nameColor + info.name + ColorManager::getReset() + " " + 
               ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
               ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset();
    } else {
        return nameColor + info.name + ColorManager::getReset() + " (" + 
               ColorManager::getSizeColor() + info.sizeFormatted + ColorManager::getReset() + ") | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
               ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset();
    }
}

void TreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
    std::cout << ColorManager::getReset();
}

TreeBuilder::Statistics TreeBuilder::getStatistics() const {
    return stats_;
}

TreeBuilder::DisplayStatistics TreeBuilder::getDisplayStatistics() const {
    DisplayStatistics result = displayStats_;
    result.hiddenObjects = hiddenObjectsCount_;
    return result;
}

const std::vector<std::string>& TreeBuilder::getTreeLines() const {
    return treeLines_;
}