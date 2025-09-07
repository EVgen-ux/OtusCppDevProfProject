#pragma once

#include "TreeBuilder.h"
#include <string>
#include <functional>
#include <regex>
#include <chrono>

class FilteredTreeBuilder : public TreeBuilder {
public:
    explicit FilteredTreeBuilder(const std::string& rootPath);
    
    // Методы для установки фильтров
    void setSizeFilter(uint64_t size, const std::string& operation = ">");
    void setDateFilter(const std::string& date, const std::string& operation = ">");
    void setNameFilter(const std::string& pattern, bool include = true);
    void setMaxDepth(size_t maxDepth);
    
    void clearFilters();
    
    void buildTree(bool showHidden = false) override;
    
private:
    struct Filter {
        enum class Type { NONE, SIZE, DATE, NAME } type = Type::NONE;
        std::string operation;
        uint64_t sizeValue = 0;
        std::chrono::system_clock::time_point dateValue;
        std::regex namePattern;
        bool include = true;
    };
    
    Filter currentFilter_;
    size_t maxDepth_;
    size_t currentDepth_;
    
    void traverseDirectory(const fs::path& path, 
                          const std::string& prefix, 
                          bool isLast,
                          bool showHidden,
                          bool isRoot = false) override;
    
    bool shouldIncludeEntry(const fs::path& path, const FileSystem::FileInfo& info) const;
    bool matchesFilter(const FileSystem::FileInfo& info) const;
};