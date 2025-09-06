#include "DepthViewTreeBuilder.h"
#include <iostream>

DepthViewTreeBuilder::DepthViewTreeBuilder(const std::string& rootPath, size_t maxDepth)
    : TreeBuilder(rootPath), maxDepth_(maxDepth), currentDepth_(0) {}

void DepthViewTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    currentDepth_ = 0;
    
    // Добавляем корневую директорию
    treeLines_.push_back("[DIR]");
    stats_.totalDirectories++;
    
    // Обходим корневую директорию
    traverseDirectory(rootPath_, "", true, showHidden, true);
}

void DepthViewTreeBuilder::traverseDirectory(const fs::path& path, 
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
    
    // Проверяем глубину
    if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
        // Достигнута максимальная глубина - показываем директорию без содержимого
        return;
    }
    
    currentDepth_++;
    
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
        currentDepth_--;
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
            // Проверяем, не достигли ли мы максимальной глубины для дочерних элементов
            if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
                // Показываем директорию с пометкой что она скрыта
                auto info = FileSystem::getFileInfo(entry.path());
                std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                                  : constants::TREE_BRANCH;
                
                treeLines_.push_back(newPrefix + connector + info.name + " [DIR] (содержимое скрыто) | " + 
                                   info.lastModified + " | " + info.permissions);
                stats_.totalDirectories++;
            } else {
                traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
            }
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                              : constants::TREE_BRANCH;
            
            treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
        }
    }
    
    currentDepth_--;
}

void DepthViewTreeBuilder::setMaxDepth(size_t maxDepth) {
    maxDepth_ = maxDepth;
}

size_t DepthViewTreeBuilder::getMaxDepth() const {
    return maxDepth_;
}