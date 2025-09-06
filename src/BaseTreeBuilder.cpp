#include "BaseTreeBuilder.h"
#include <functional>

#define UNUSED(x) (void)(x)

BaseTreeBuilder::BaseTreeBuilder(const std::string& rootPath) 
    : rootPath_(rootPath) {}

void BaseTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    
    // Добавляем корневую директорию
    auto rootInfo = FileSystem::getFileInfo(rootPath_);
    treeLines_.push_back("[DIR]");
    stats_.totalDirectories++;
    
    // Обходим содержимое корневой директории (но не саму корневую директорию)
    auto filter = [](const fs::directory_entry& entry, bool showHidden) {
        return !FileSystem::isHidden(entry.path()) || showHidden;
    };
    
    auto sort = [](const fs::directory_entry& a, const fs::directory_entry& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.path().filename().string() < b.path().filename().string();
    };
    
    processDirectoryEntries(rootPath_, filter, sort, showHidden, "", true);
}

void BaseTreeBuilder::traverseDirectory(const fs::path& path, 
                                     const std::string& prefix, 
                                     bool isLast,
                                     bool showHidden,
                                     bool isRoot) {
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH 
                                     : constants::TREE_BRANCH;
        
        treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
        stats_.totalDirectories++;
    }
    
    // Рекурсивно обходим содержимое директории
    std::string newPrefix = prefix;
    if (isLast) {
        newPrefix += constants::TREE_SPACE;
    } else {
        newPrefix += constants::TREE_VERTICAL;
    }
    
    std::vector<fs::directory_entry> entries;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (!FileSystem::isHidden(entry.path()) || showHidden) {
                entries.push_back(entry);
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
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                              : constants::TREE_BRANCH;
            
            treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
        }
    }
}

bool BaseTreeBuilder::shouldIncludeFile(const fs::path& path, bool showHidden) const {
    return !FileSystem::isHidden(path) || showHidden;
}

std::string BaseTreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                         const std::string& connector) const {
    UNUSED(connector);
    
    if (info.isDirectory) {
        return info.name + " [DIR] | " + info.lastModified + " | " + info.permissions;
    } else {
        return info.name + " (" + info.sizeFormatted + ") | " + 
               info.lastModified + " | " + info.permissions;
    }
}

void BaseTreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
}

ITreeBuilder::Statistics BaseTreeBuilder::getStatistics() const {
    return stats_;
}

const std::vector<std::string>& BaseTreeBuilder::getTreeLines() const {
    return treeLines_;
}