#include "DepthViewTreeBuilder.h"
#include "ColorManager.h" // Добавьте этот include
#include <iostream>

DepthViewTreeBuilder::DepthViewTreeBuilder(const std::string& rootPath, size_t maxDepth)
    : TreeBuilder(rootPath), maxDepth_(maxDepth), currentDepth_(0) {}

void DepthViewTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    currentDepth_ = 0;
    
    // Добавляем корневую директорию с использованием ColorManager
    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    
    // Если глубина не ограничена, используем базовую реализацию
    if (maxDepth_ == 0) {
        TreeBuilder::buildTree(showHidden);
        return;
    }
    
    // Обходим корневую директорию с ограничением глубина
    traverseDirectory(rootPath_, "", true, showHidden, true);
}

void DepthViewTreeBuilder::traverseDirectory(const fs::path& path, 
                                           const std::string& prefix, 
                                           bool isLast,
                                           bool showHidden,
                                           bool isRoot) {
    // Для не корневых директорий добавляем в дерево и статистику
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH 
                                     : constants::TREE_BRANCH;
        
        std::string nameColor = FileSystem::getFileColor(info);
        treeLines_.push_back(prefix + connector + nameColor + info.name + ColorManager::getReset() + " " + 
                           ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " | " + 
                           ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
                           ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset());
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
    }
    
    // Проверяем глубину
    if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
        // Достигнута максимальная глубина - показываем директорию без содержимого
        if (!isRoot) {
            displayStats_.hiddenByDepth++;
        }
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
                std::string nameColor = FileSystem::getFileColor(info);
                
                treeLines_.push_back(newPrefix + connector + nameColor + info.name + ColorManager::getReset() + " " + 
                                   ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " " + 
                                   ColorManager::getHiddenContentColor() + "(содержимое скрыто)" + ColorManager::getReset() + " | " + 
                                   ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
                                   ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset());
                stats_.totalDirectories++;
                displayStats_.displayedDirectories++;
                displayStats_.hiddenByDepth++;
            } else {
                traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
            }
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                              : constants::TREE_BRANCH;
            std::string nameColor = FileSystem::getFileColor(info);
            
            treeLines_.push_back(newPrefix + connector + nameColor + info.name + ColorManager::getReset() + " (" + 
                               ColorManager::getSizeColor() + info.sizeFormatted + ColorManager::getReset() + ") | " + 
                               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
                               ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset());
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
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