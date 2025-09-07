#pragma once

#include "TreeBuilder.h"
#include <string>
#include <functional>
#include <regex>
#include <chrono>
#include <vector>

class FilteredTreeBuilder : public TreeBuilder {
public:
    explicit FilteredTreeBuilder(const std::string& rootPath);
    
    // Методы для добавления фильтров (множественные)
    void addSizeFilter(uint64_t size, const std::string& operation = ">");
    void addDateFilter(const std::string& date, const std::string& operation = ">");
    void addNameFilter(const std::string& pattern, bool include = true);
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
    
    std::vector<Filter> filters_; // Множественные фильтры
    size_t maxDepth_;
    size_t currentDepth_;
    
    void traverseDirectory(const fs::path& path, 
                          const std::string& prefix, 
                          bool isLast,
                          bool showHidden,
                          bool isRoot = false) override;
    
    bool shouldIncludeEntry(const fs::path& path, const FileSystem::FileInfo& info) const;
    bool matchesAllFilters(const FileSystem::FileInfo& info) const;
    bool matchesSingleFilter(const FileSystem::FileInfo& info, const Filter& filter) const;
    
    std::string wildcardToRegex(const std::string& pattern) const;
};