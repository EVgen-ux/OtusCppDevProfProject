#include "TreeBuilder.h"
#include "FileSystem.h"
#include "constants.h"
#include <iostream>
#include <algorithm>

TreeBuilder::TreeBuilder(const std::string& rootPath) : rootPath_(rootPath) {}

void TreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics();
    
    if (!fs::exists(rootPath_)) {
        throw std::runtime_error("Директория не существует: " + rootPath_.string());
    }
    
    if (!fs::is_directory(rootPath_)) {
        throw std::runtime_error("Указанный путь не является директорией: " + rootPath_.string());
    }
    
    auto info = FileSystem::getFileInfo(rootPath_);
    treeLines_.push_back(rootPath_.filename().string() + " [DIR]");
    stats_.totalDirectories++;
    
    traverseDirectory(rootPath_, "", true, showHidden);
}

void TreeBuilder::traverseDirectory(const fs::path& path, 
                                   const std::string& prefix, 
                                   bool isLast,
                                   bool showHidden) {
    std::vector<fs::path> entries;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (shouldIncludeFile(entry.path(), showHidden)) {
                entries.push_back(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        treeLines_.push_back(prefix + constants::TREE_LAST_BRANCH + "[Ошибка доступа]");
        return;
    }
    
    // Сортировка: сначала директории, потом файлы
    std::sort(entries.begin(), entries.end(), [](const fs::path& a, const fs::path& b) {
        bool aIsDir = fs::is_directory(a);
        bool bIsDir = fs::is_directory(b);
        if (aIsDir != bIsDir) return aIsDir > bIsDir;
        return a.filename() < b.filename();
    });
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool isLastEntry = (i == entries.size() - 1);
        
        auto info = FileSystem::getFileInfo(entry);
        std::string connector = isLastEntry ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        std::string newPrefix = prefix + (isLast ? constants::TREE_SPACE : constants::TREE_VERTICAL);
        
        std::string line = prefix + connector + info.name;
        if (info.isDirectory) {
            line += " [DIR]";
            stats_.totalDirectories++;
        } else {
            line += " (" + info.sizeFormatted + ")";
            stats_.totalFiles++;
            stats_.totalSize += info.size;
        }
        
        line += " | " + info.lastModified + " | " + info.permissions;
        treeLines_.push_back(line);
        
        if (info.isDirectory) {
            traverseDirectory(entry, newPrefix, isLastEntry, showHidden);
        }
    }
}

bool TreeBuilder::shouldIncludeFile(const fs::path& path, bool showHidden) const {
    if (!showHidden && FileSystem::isHidden(path)) {
        return false;
    }
    return true;
}

void TreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
}

TreeBuilder::Statistics TreeBuilder::getStatistics() const {
    return stats_;
}