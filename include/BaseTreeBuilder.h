#pragma once

#include "ITreeBuilder.h"
#include "FileSystem.h"
#include "constants.h"
#include <memory>
#include <functional>
#include <vector>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

class BaseTreeBuilder : public ITreeBuilder {
public:
    explicit BaseTreeBuilder(const std::string& rootPath);
    virtual ~BaseTreeBuilder() = default;
    
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    Statistics getStatistics() const override;
    DisplayStatistics getDisplayStatistics() const override;
    const std::vector<std::string>& getTreeLines() const override;
    
    std::vector<std::string>& getTreeLines() { return treeLines_; }
    
protected:
    fs::path rootPath_;
    Statistics stats_;
    DisplayStatistics displayStats_;
    std::vector<std::string> treeLines_;
    size_t hiddenObjectsCount_ = 0;
    
    virtual void traverseDirectory(const fs::path& path, 
                                 const std::string& prefix, 
                                 bool isLast,
                                 bool showHidden,
                                 bool isRoot = false);
    
    virtual bool shouldIncludeFile(const fs::path& path, bool showHidden) const;
    virtual std::string formatTreeLine(const FileSystem::FileInfo& info, 
                                     const std::string& connector) const;
    
    template<typename FilterFunc, typename SortFunc>
    void processDirectoryEntries(const fs::path& path, 
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
        return;
    }
    
    std::sort(entries.begin(), entries.end(), sort);
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        
        std::string newPrefix = prefix;
        if (isLast) {
            newPrefix += constants::TREE_SPACE;
        } else {
            newPrefix += constants::TREE_VERTICAL;
        }
        
        auto info = FileSystem::getFileInfo(entry.path());
        std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                          : constants::TREE_BRANCH;
        
        if (entry.is_directory()) {
            // Для директорий просто вызываем traverseDirectory
            traverseDirectory(entry.path(), prefix, entryIsLast, showHidden);
            
            // Обновляем статистику
            stats_.totalDirectories++;
            displayStats_.displayedDirectories++;
        } else {
            treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
}
};