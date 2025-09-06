#include "BaseTreeBuilder.h"
#include <algorithm>
#include <iostream>

BaseTreeBuilder::BaseTreeBuilder(const std::string& rootPath) 
    : rootPath_(rootPath) {
    stats_ = Statistics{0, 0, 0};
}

void BaseTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    
    if (!fs::exists(rootPath_)) {
        throw std::runtime_error("Path does not exist: " + rootPath_.string());
    }
    
    auto info = FileSystem::getFileInfo(rootPath_);
    treeLines_.push_back(info.name + " (directory)");
    stats_.totalDirectories++;
    
    traverseDirectory(rootPath_, "", true, showHidden);
}

void BaseTreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
    
    std::cout << "\nStatistics:\n";
    std::cout << "Directories: " << stats_.totalDirectories << "\n";
    std::cout << "Files: " << stats_.totalFiles << "\n";
    std::cout << "Total size: " << FileSystem::formatSize(stats_.totalSize) << "\n";
}

ITreeBuilder::Statistics BaseTreeBuilder::getStatistics() const {
    return stats_;
}

const std::vector<std::string>& BaseTreeBuilder::getTreeLines() const {
    return treeLines_;
}

void BaseTreeBuilder::traverseDirectory(const fs::path& path, 
                                      const std::string& prefix, 
                                      bool isLast,
                                      bool showHidden) {
    auto filter = [](const fs::directory_entry& entry, bool showHidden) {
        return showHidden || !FileSystem::isHidden(entry.path());
    };
    
    auto sort = [](const fs::directory_entry& a, const fs::directory_entry& b) {
        // Сначала директории, потом файлы
        bool aIsDir = a.is_directory();
        bool bIsDir = b.is_directory();
        
        if (aIsDir != bIsDir) {
            return aIsDir > bIsDir; // Директории first
        }
        return a.path().filename() < b.path().filename();
    };
    
    processDirectoryEntries(path, filter, sort, showHidden, prefix, isLast);
}

bool BaseTreeBuilder::shouldIncludeFile(const fs::path& path, bool showHidden) const {
    return showHidden || !FileSystem::isHidden(path);
}

std::string BaseTreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                          const std::string& /*connector*/) const {
    return info.name + (info.isDirectory ? " (directory)" : " (" + info.sizeFormatted + ")");
}