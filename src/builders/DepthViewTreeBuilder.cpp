#include "DepthViewTreeBuilder.h"
#include <iostream>
#include <algorithm> 

DepthViewTreeBuilder::DepthViewTreeBuilder(const std::string& rootPath, size_t maxDepth)
    : TreeBuilder(rootPath), maxDepth_(maxDepth), currentDepth_(0) {}

void DepthViewTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    currentDepth_ = 0;
    
    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    
    traverseDirectory(rootPath_, "", true, showHidden, true);
}

void DepthViewTreeBuilder::traverseDirectory(const fs::path& path, 
                                           const std::string& prefix, 
                                           bool isLast,
                                           bool showHidden,
                                           bool isRoot) {
    if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
        if (!isRoot) {
            displayStats_.hiddenByDepth++;
        }
        return;
    }
    
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        
        treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
    }
    
    currentDepth_++;
    
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
        currentDepth_--;
        return;
    }
    
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
            if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
                auto info = FileSystem::getFileInfo(entry.path());
                std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
                
                treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector) + " " + 
                                   ColorManager::getHiddenContentColor() + "(содержимое скрыто)" + ColorManager::getReset());
                stats_.totalDirectories++;
                displayStats_.displayedDirectories++;
                displayStats_.hiddenByDepth++;
            } else {
                traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
            }
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
    
    currentDepth_--;
}

std::string DepthViewTreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
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

void DepthViewTreeBuilder::setMaxDepth(size_t maxDepth) {
    maxDepth_ = maxDepth;
}

size_t DepthViewTreeBuilder::getMaxDepth() const {
    return maxDepth_;
}