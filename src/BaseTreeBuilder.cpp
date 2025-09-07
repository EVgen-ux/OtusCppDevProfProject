#include "BaseTreeBuilder.h"
#include "ColorManager.h" 
#include <functional>

#define UNUSED(x) (void)(x)

BaseTreeBuilder::BaseTreeBuilder(const std::string& rootPath) 
    : rootPath_(rootPath) {}

void BaseTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    
    // Добавляем корневую директорию с использованием ColorManager
    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    
    // Обходим содержимое корневой директории
    traverseDirectory(rootPath_, "", true, showHidden, true);
}

void BaseTreeBuilder::traverseDirectory(const fs::path& path, 
                                     const std::string& prefix, 
                                     bool isLast,
                                     bool showHidden,
                                     bool isRoot) {
    // Для не корневых директорий добавляем в дерево и статистику
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH 
                                     : constants::TREE_BRANCH;
        
        treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
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
            // Директории учитываются в traverseDirectory при входе
            traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                              : constants::TREE_BRANCH;
            
            treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
}


bool BaseTreeBuilder::shouldIncludeFile(const fs::path& path, bool showHidden) const {
    return !FileSystem::isHidden(path) || showHidden;
}

std::string BaseTreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                         const std::string& connector) const {
    UNUSED(connector);
    
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

void BaseTreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
    std::cout << ColorManager::getReset(); // Сбрасываем цвет в конце
}

ITreeBuilder::Statistics BaseTreeBuilder::getStatistics() const {
    return stats_;
}

ITreeBuilder::DisplayStatistics BaseTreeBuilder::getDisplayStatistics() const {
    return displayStats_;
}

const std::vector<std::string>& BaseTreeBuilder::getTreeLines() const {
    return treeLines_;
}