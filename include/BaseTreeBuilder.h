#pragma once

#include "ITreeBuilder.h"
#include "FileSystem.h"
#include "constants.h"
#include <memory>
#include <functional>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class BaseTreeBuilder : public ITreeBuilder {
public:
    explicit BaseTreeBuilder(const std::string& rootPath);
    virtual ~BaseTreeBuilder() = default;
    
    // Основные методы
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    Statistics getStatistics() const override;
    const std::vector<std::string>& getTreeLines() const override;
    
    // Неконстантный метод для модификации
    std::vector<std::string>& getTreeLines() { return treeLines_; }
    
protected:
    fs::path rootPath_;
    Statistics stats_;
    std::vector<std::string> treeLines_;
    
    // Виртуальные методы для переопределения
    virtual void traverseDirectory(const fs::path& path, 
                                 const std::string& prefix, 
                                 bool isLast,
                                 bool showHidden);
    
    virtual bool shouldIncludeFile(const fs::path& path, bool showHidden) const;
    virtual std::string formatTreeLine(const FileSystem::FileInfo& info, 
                                     const std::string& connector) const;
    
    // Шаблонный метод для обработки записей
    template<typename FilterFunc, typename SortFunc>
    void processDirectoryEntries(const fs::path& path, 
                               FilterFunc filter, 
                               SortFunc sort,
                               bool showHidden,
                               const std::string& prefix,
                               bool isLast);
};

// Реализация шаблонной функции в заголовочном файле
template<typename FilterFunc, typename SortFunc>
void BaseTreeBuilder::processDirectoryEntries(const fs::path& path, 
                                           FilterFunc filter, 
                                           SortFunc sort,
                                           bool showHidden,
                                           const std::string& prefix,
                                           bool isLast) {
    std::vector<fs::directory_entry> entries;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (filter(entry, showHidden)) {
                entries.push_back(entry);
            }
        }
    } catch (const fs::filesystem_error&) {
        return; // Пропускаем директории, к которым нет доступа
    }
    
    // Сортировка записей
    std::sort(entries.begin(), entries.end(), sort);
    
    // Обработка записей
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        
        std::string newPrefix = prefix;
        if (isLast) {
            newPrefix += constants::TREE_SPACE;
        } else {
            newPrefix += constants::TREE_VERTICAL;
        }
        
        if (entry.is_directory()) {
            traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden);
        } else {
            auto info = FileSystem::getFileInfo(entry.path());
            std::string connector = entryIsLast ? std::string(constants::TREE_LAST_BRANCH) 
                                              : std::string(constants::TREE_BRANCH);
            treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
        }
    }
}