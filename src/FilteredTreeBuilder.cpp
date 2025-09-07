#include "FilteredTreeBuilder.h"
#include "FileSystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>

FilteredTreeBuilder::FilteredTreeBuilder(const std::string& rootPath) 
    : TreeBuilder(rootPath), maxDepth_(0), currentDepth_(0) {
    currentFilter_.type = Filter::Type::NONE;
}

void FilteredTreeBuilder::setSizeFilter(uint64_t size, const std::string& operation) {
    currentFilter_.type = Filter::Type::SIZE;
    currentFilter_.sizeValue = size;
    currentFilter_.operation = operation;
}

void FilteredTreeBuilder::setDateFilter(const std::string& date, const std::string& operation) {
    currentFilter_.type = Filter::Type::DATE;
    currentFilter_.operation = operation;
    
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
        currentFilter_.dateValue = std::chrono::system_clock::from_time_t(tt);
    } else {
        currentFilter_.type = Filter::Type::NONE;
        std::cerr << "Ошибка: неверный формат даты. Используйте YYYY-MM-DD или YYYY-MM-DD HH:MM:SS" << std::endl;
    }
}

void FilteredTreeBuilder::setNameFilter(const std::string& pattern, bool include) {
    currentFilter_.type = Filter::Type::NAME;
    try {
        // Преобразуем wildcard pattern в regex
        std::string regexPattern;
        for (char c : pattern) {
            switch (c) {
                case '*':
                    regexPattern += ".*"; // * -> .*
                    break;
                case '?':
                    regexPattern += '.';  // ? -> .
                    break;
                case '.':
                    regexPattern += "\\."; // . -> \.
                    break;
                case '\\':
                    regexPattern += "\\\\"; // \ -> \\
                    break;
                case '+':
                    regexPattern += "\\+"; // + -> \+
                    break;
                case '^':
                    regexPattern += "\\^"; // ^ -> \^
                    break;
                case '$':
                    regexPattern += "\\$"; // $ -> \$
                    break;
                case '|':
                    regexPattern += "\\|"; // | -> \|
                    break;
                case '(':
                    regexPattern += "\\("; // ( -> \(
                    break;
                case ')':
                    regexPattern += "\\)"; // ) -> \)
                    break;
                case '[':
                    regexPattern += "\\["; // [ -> \[
                    break;
                case ']':
                    regexPattern += "\\]"; // ] -> \]
                    break;
                case '{':
                    regexPattern += "\\{"; // { -> \{
                    break;
                case '}':
                    regexPattern += "\\}"; // } -> \}
                    break;
                default:
                    regexPattern += c;
                    break;
            }
        }
        
        currentFilter_.namePattern = std::regex(regexPattern, 
            std::regex_constants::icase | std::regex_constants::optimize);
        currentFilter_.include = include;
        
        // Отладочная печать (можно убрать после тестирования)
        std::cout << "Шаблон: " << pattern << " -> Regex: " << regexPattern << std::endl;
        
    } catch (const std::regex_error& e) {
        currentFilter_.type = Filter::Type::NONE;
        std::cerr << "Ошибка в шаблоне имени: " << e.what() << std::endl;
    }
}

void FilteredTreeBuilder::setMaxDepth(size_t maxDepth) {
    maxDepth_ = maxDepth;
}

void FilteredTreeBuilder::clearFilters() {
    currentFilter_.type = Filter::Type::NONE;
}

void FilteredTreeBuilder::buildTree(bool showHidden) {
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    currentDepth_ = 0;
    
    treeLines_.push_back("[DIR]");
    
    // Если установлена максимальная глубина, используем наш обход с ограничением
    if (maxDepth_ > 0) {
        traverseDirectory(rootPath_, "", true, showHidden, true);
    } else {
        // Иначе используем базовую реализацию
        TreeBuilder::buildTree(showHidden);
    }
}

bool FilteredTreeBuilder::shouldIncludeEntry(const fs::path& path, const FileSystem::FileInfo& info) const {
    // Всегда включаем директории при фильтрации
    if (info.isDirectory) {
        return true;
    }
    
    return matchesFilter(info);
}

bool FilteredTreeBuilder::matchesFilter(const FileSystem::FileInfo& info) const {
    if (currentFilter_.type == Filter::Type::NONE) {
        return true;
    }
    
    switch (currentFilter_.type) {
        case Filter::Type::SIZE:
            if (currentFilter_.operation == ">") {
                return info.size > currentFilter_.sizeValue;
            } else if (currentFilter_.operation == "<") {
                return info.size < currentFilter_.sizeValue;
            } else if (currentFilter_.operation == "==") {
                return info.size == currentFilter_.sizeValue;
            } else if (currentFilter_.operation == ">=") {
                return info.size >= currentFilter_.sizeValue;
            } else if (currentFilter_.operation == "<=") {
                return info.size <= currentFilter_.sizeValue;
            }
            break;
            
        case Filter::Type::DATE: {
            // Преобразуем строку времени файла обратно в time_point для сравнения
            std::tm tm = {};
            std::istringstream ss(info.lastModified);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (!ss.fail()) {
                std::time_t tt = std::mktime(&tm);
                auto fileTime = std::chrono::system_clock::from_time_t(tt);
                
                if (currentFilter_.operation == ">") {
                    return fileTime > currentFilter_.dateValue;
                } else if (currentFilter_.operation == "<") {
                    return fileTime < currentFilter_.dateValue;
                } else if (currentFilter_.operation == "==") {
                    return fileTime == currentFilter_.dateValue;
                }
            }
            break;
        }
            
        case Filter::Type::NAME: {
            bool matches = std::regex_match(info.name, currentFilter_.namePattern);
            return currentFilter_.include ? matches : !matches;
        }
            
        case Filter::Type::NONE:
        default:
            break;
    }
    
    return true;
}

void FilteredTreeBuilder::traverseDirectory(const fs::path& path, 
                                          const std::string& prefix, 
                                          bool isLast,
                                          bool showHidden,
                                          bool isRoot) {
    // Проверяем глубину
    if (maxDepth_ > 0 && currentDepth_ >= maxDepth_) {
        if (!isRoot) {
            displayStats_.hiddenByDepth++;
        }
        return;
    }
    
    // Для не корневых директорий добавляем в дерево и статистику
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        if (shouldIncludeEntry(path, info)) {
            std::string connector = isLast ? constants::TREE_LAST_BRANCH 
                                         : constants::TREE_BRANCH;
            
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
            if (!FileSystem::isHidden(entry.path()) || showHidden) {
                entries.push_back(entry);
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
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                              : constants::TREE_BRANCH;
            
            treeLines_.push_back(newPrefix + connector + formatTreeLine(info, connector));
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
    
    currentDepth_--;
}