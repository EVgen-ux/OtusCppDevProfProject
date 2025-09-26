#include "FilteredTreeBuilder.h"
#include <iostream>
#include <sstream>
#include <iomanip>

FilteredTreeBuilder::FilteredTreeBuilder(const std::string& rootPath) 
    : TreeBuilder(rootPath), maxDepth_(0), currentDepth_(0) {}

void FilteredTreeBuilder::addSizeFilter(uint64_t size, const std::string& operation) {
    Filter filter;
    filter.type = Filter::Type::SIZE;
    filter.sizeValue = size;
    filter.operation = operation;
    filters_.push_back(filter);
    
    std::cout << "Добавлен фильтр размера: " << operation << " " 
              << FileSystem::formatSize(size) << std::endl;
}

void FilteredTreeBuilder::addDateFilter(const std::string& date, const std::string& operation) {
    Filter filter;
    filter.type = Filter::Type::DATE;
    filter.operation = operation;
    
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        ss.clear();
        ss.str(date);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    }
    
    if (!ss.fail()) {
        std::time_t tt = std::mktime(&tm);
        filter.dateValue = std::chrono::system_clock::from_time_t(tt);
        filters_.push_back(filter);
        
        std::cout << "Добавлен фильтр даты: " << operation << " " << date << std::endl;
    } else {
        std::cerr << "Ошибка: неверный формат даты. Используйте YYYY-MM-DD или YYYY-MM-DD HH:MM:SS" << std::endl;
    }
}

void FilteredTreeBuilder::addNameFilter(const std::string& pattern, bool include) {
    Filter filter;
    filter.type = Filter::Type::NAME;
    filter.include = include;
    
    try {
        std::string regexPattern = wildcardToRegex(pattern);
        filter.namePattern = std::regex(regexPattern, 
            std::regex_constants::icase | std::regex_constants::optimize);
        filters_.push_back(filter);
        
        std::string filterType = include ? "включения" : "исключения";
        std::cout << "Добавлен фильтр имени (" << filterType << "): " 
                  << pattern << " -> " << regexPattern << std::endl;
        
    } catch (const std::regex_error& e) {
        std::cerr << "Ошибка в шаблоне имени: " << e.what() << std::endl;
    }
}

std::string FilteredTreeBuilder::wildcardToRegex(const std::string& pattern) const {
    std::string regexPattern;
    for (char c : pattern) {
        switch (c) {
            case '*': regexPattern += ".*"; break;
            case '?': regexPattern += '.'; break;
            case '.': regexPattern += "\\."; break;
            case '\\': regexPattern += "\\\\"; break;
            case '+': regexPattern += "\\+"; break;
            case '^': regexPattern += "\\^"; break;
            case '$': regexPattern += "\\$"; break;
            case '|': regexPattern += "\\|"; break;
            case '(': regexPattern += "\\("; break;
            case ')': regexPattern += "\\)"; break;
            case '[': regexPattern += "\\["; break;
            case ']': regexPattern += "\\]"; break;
            case '{': regexPattern += "\\{"; break;
            case '}': regexPattern += "\\}"; break;
            default: regexPattern += c; break;
        }
    }
    return regexPattern;
}

void FilteredTreeBuilder::setMaxDepth(size_t maxDepth) {
    maxDepth_ = maxDepth;
}

void FilteredTreeBuilder::clearFilters() {
    filters_.clear();
}

void FilteredTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    currentDepth_ = 0;
    
    treeLines_.push_back("[DIR]");
    
    if (maxDepth_ > 0) {
        traverseDirectory(rootPath_, "", true, showHidden, true);
    } else {
        TreeBuilder::buildTree(showHidden);
    }
}

bool FilteredTreeBuilder::shouldIncludeEntry(const fs::path& path, const FileSystem::FileInfo& info) const {
    if (info.isDirectory) {
        return true;
    }
    return matchesAllFilters(info);
}

bool FilteredTreeBuilder::matchesAllFilters(const FileSystem::FileInfo& info) const {
    if (filters_.empty()) {
        return true; 
    }
    
    for (const auto& filter : filters_) {
        if (!matchesSingleFilter(info, filter)) {
            return false;
        }
    }
    return true;
}

bool FilteredTreeBuilder::matchesSingleFilter(const FileSystem::FileInfo& info, const Filter& filter) const {
    switch (filter.type) {
        case Filter::Type::SIZE:
            if (filter.operation == ">") return info.size > filter.sizeValue;
            else if (filter.operation == "<") return info.size < filter.sizeValue;
            else if (filter.operation == "==") return info.size == filter.sizeValue;
            else if (filter.operation == ">=") return info.size >= filter.sizeValue;
            else if (filter.operation == "<=") return info.size <= filter.sizeValue;
            break;
            
        case Filter::Type::DATE: {
            std::tm tm = {};
            std::istringstream ss(info.lastModified);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (!ss.fail()) {
                std::time_t tt = std::mktime(&tm);
                auto fileTime = std::chrono::system_clock::from_time_t(tt);
                
                if (filter.operation == ">") return fileTime > filter.dateValue;
                else if (filter.operation == "<") return fileTime < filter.dateValue;
                else if (filter.operation == "==") return fileTime == filter.dateValue;
            }
            break;
        }
            
        case Filter::Type::NAME: {
            bool matches = std::regex_match(info.name, filter.namePattern);
            return filter.include ? matches : !matches;
        }
            
        default: break;
    }
    
    return true;
}

void FilteredTreeBuilder::traverseDirectory(const fs::path& path, 
                                          const std::string& prefix, 
                                          bool isLast,
                                          bool showHidden,
                                          bool isRoot) {
    if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
        if (!isRoot) {
            displayStats_.hiddenByDepth++;
        }
        return;
    }
    
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        if (shouldIncludeEntry(path, info)) {
            std::string connector = isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
            treeLines_.push_back(prefix + connector + formatTreeLine(info, connector));
            stats_.totalDirectories++;
            displayStats_.displayedDirectories++;
        }
    }
    
    currentDepth_++;
    
    std::string newPrefix = prefix;
    if (isLast) {
        newPrefix += constants::TREE_SPACE;
    } else {
        newPrefix += constants::TREE_VERTICAL;
    }
    
    std::vector<fs::directory_entry> entries;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            bool isHidden = FileSystem::isHidden(entry.path());
            
            if (!isHidden || showHidden) {
                entries.push_back(entry);
            } else {
                hiddenObjectsCount_++;
            }
        }
    } catch (const fs::filesystem_error&) {
        currentDepth_--;
        return;
    }
    
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.path().filename().string() < b.path().filename().string();
    });
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        auto info = FileSystem::getFileInfo(entry.path());
        
        if (!shouldIncludeEntry(entry.path(), info)) {
            continue;
        }
        
        if (entry.is_directory()) {
            traverseDirectory(entry.path(), newPrefix, entryIsLast, showHidden, false);
        } else {
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
            std::string nameColor = FileSystem::getFileColor(info);
treeLines_.push_back(prefix + connector + nameColor + info.name + ColorManager::getReset() + " " + 
                   ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " | " + 
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