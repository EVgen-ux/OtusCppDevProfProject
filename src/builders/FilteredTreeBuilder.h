#pragma once
#include "TreeBuilder.h"
#include <string>
#include <regex>
#include <chrono>
#include <vector>

class FilteredTreeBuilder : public TreeBuilder {
public:
    explicit FilteredTreeBuilder(const std::string& rootPath);
    
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
    
    std::vector<Filter> filters_;
    size_t maxDepth_;
    size_t currentDepth_;
    
    void traverseDirectory(const std::filesystem::path& path, 
                          const std::string& prefix, 
                          bool isLast,
                          bool showHidden,
                          bool isRoot = false) override;
    
    bool shouldIncludeEntry(const std::filesystem::path& path, const FileSystem::FileInfo& info) const;
    bool matchesAllFilters(const FileSystem::FileInfo& info) const;
    bool matchesSingleFilter(const FileSystem::FileInfo& info, const Filter& filter) const;
    std::string wildcardToRegex(const std::string& pattern) const;
    std::string formatTreeLine(const FileSystem::FileInfo& info, const std::string& connector) const override;
};