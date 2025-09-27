#include "MultiThreadedTreeBuilder.h"
#include <iostream>
#include <algorithm>
#include <future>

namespace fs = std::filesystem;

MultiThreadedTreeBuilder::MultiThreadedTreeBuilder(const std::string& rootPath, size_t threadCount)
    : TreeBuilder(rootPath), threadCount_(threadCount) {
    
    if (threadCount_ == 0) {
        unsigned int hwThreads = std::thread::hardware_concurrency();
        threadCount_ = (hwThreads == 0) ? 1 : static_cast<size_t>(hwThreads);
    }
}

MultiThreadedTreeBuilder::~MultiThreadedTreeBuilder() {
    stopProcessing_ = true;
}

void MultiThreadedTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    hiddenObjectsCount_ = 0;
    stopProcessing_ = false;
    
    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    traverseDirectoryHybrid(rootPath_, "", true, showHidden, true);
}

void MultiThreadedTreeBuilder::traverseDirectoryHybrid(const fs::path& path, 
                                                     const std::string& prefix, 
                                                     bool isLast,
                                                     bool showHidden,
                                                     bool isRoot) {
    if (stopProcessing_) return;
    
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
    
    std::vector<std::future<FileSystem::FileInfo>> fileFutures;
    std::vector<fs::directory_entry> fileEntries;
    
    for (const auto& entry : entries) {
        if (!entry.is_directory()) {
            fileEntries.push_back(entry);
            fileFutures.push_back(std::async(std::launch::async, [entry]() {
                return FileSystem::getFileInfo(entry.path());
            }));
        }
    }
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        
        if (entry.is_directory()) {
            traverseDirectoryHybrid(entry.path(), newPrefix, entryIsLast, showHidden, false);
        }
    }
    
     for (size_t i = 0; i < fileEntries.size(); ++i) {
        auto info = fileFutures[i].get();
        bool entryIsLast = (i == fileEntries.size() - 1);
        std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        
        treeLines_.push_back(newPrefix + connector + 
            FileSystem::getFileColor(info) + info.name + ColorManager::getReset() + " (" + 
            ColorManager::getSizeColor() + info.sizeFormatted + ColorManager::getReset() + ") | " + 
            ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
            ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset());
        
        stats_.totalFiles++;
        stats_.totalSize += info.size;
        displayStats_.displayedFiles++;
        displayStats_.displayedSize += info.size;
    }
}