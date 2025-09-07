#include "CommandLineParser.h"
#include "DepthViewTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "FileSystem.h"
#include "constants.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>

uint64_t parseSize(const std::string& sizeStr) {
    size_t multiplier = 1;
    std::string numStr = sizeStr;
    
    numStr.erase(std::remove_if(numStr.begin(), numStr.end(), ::isspace), numStr.end());
    
    std::string lowerStr = sizeStr;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    
    if (lowerStr.find("kb") != std::string::npos) {
        multiplier = constants::KB;
        numStr = sizeStr.substr(0, sizeStr.size() - 2);
    } else if (lowerStr.find("mb") != std::string::npos) {
        multiplier = constants::MB;
        numStr = sizeStr.substr(0, sizeStr.size() - 2);
    } else if (lowerStr.find("gb") != std::string::npos) {
        multiplier = constants::GB;
        numStr = sizeStr.substr(0, sizeStr.size() - 2);
    } else if (lowerStr.find("tb") != std::string::npos) {
        multiplier = constants::TB;
        numStr = sizeStr.substr(0, sizeStr.size() - 2);
    } else if (lowerStr.find('b') != std::string::npos && lowerStr.find("kb") == std::string::npos &&
               lowerStr.find("mb") == std::string::npos && lowerStr.find("gb") == std::string::npos &&
               lowerStr.find("tb") == std::string::npos) {
        multiplier = 1;
        numStr = sizeStr.substr(0, sizeStr.size() - 1);
    }
    
    numStr.erase(std::remove_if(numStr.begin(), numStr.end(), ::isspace), numStr.end());
    
    try {
        double size = std::stod(numStr);
        return static_cast<uint64_t>(size * multiplier);
    } catch (const std::exception&) {
        throw std::runtime_error("Неверный формат размера: " + sizeStr);
    }
}

CommandLineOptions CommandLineParser::parse(int argc, char* argv[]) {
    CommandLineOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-a" || arg == "--all") {
            options.showHidden = true;
        } else if (arg == "-L" || arg == "--level") {
            if (i + 1 < argc) {
                try {
                    options.maxDepth = std::stoul(argv[++i]);
                } catch (const std::exception&) {
                    throw std::runtime_error("Неверное значение для глубины: " + std::string(argv[i]));
                }
            } else {
                throw std::runtime_error("Опция -L требует значения");
            }
        } else if (arg == "-s" || arg == "--size") {
            if (i + 1 < argc) {
                std::string sizeArg = argv[++i];
                std::string op;
                std::string sizeStr;
                
                size_t spacePos = sizeArg.find(' ');
                if (spacePos != std::string::npos) {
                    op = sizeArg.substr(0, spacePos);
                    sizeStr = sizeArg.substr(spacePos + 1);
                } else {
                    for (size_t j = 0; j < sizeArg.size(); ++j) {
                        if (isdigit(sizeArg[j])) {
                            op = sizeArg.substr(0, j);
                            sizeStr = sizeArg.substr(j);
                            break;
                        }
                    }
                    if (op.empty()) {
                        throw std::runtime_error("Неверный формат размера: " + sizeArg);
                    }
                }
                
                Filter filter;
                filter.type = Filter::Type::SIZE;
                filter.operation = op;
                filter.sizeValue = parseSize(sizeStr);
                options.filters.push_back(filter);
            } else {
                throw std::runtime_error("Опция -s требует значения");
            }
        } else if (arg == "-d" || arg == "--date") {
            if (i + 1 < argc) {
                std::string dateArg = argv[++i];
                std::string op;
                std::string dateStr;
                
                size_t spacePos = dateArg.find(' ');
                if (spacePos != std::string::npos) {
                    op = dateArg.substr(0, spacePos);
                    dateStr = dateArg.substr(spacePos + 1);
                } else {
                    throw std::runtime_error("Неверный формат даты: " + dateArg);
                }
                
                Filter filter;
                filter.type = Filter::Type::DATE;
                filter.operation = op;
                filter.dateValue = dateStr;
                options.filters.push_back(filter);
            } else {
                throw std::runtime_error("Опция -d требует значения");
            }
        } else if (arg == "-n" || arg == "--name") {
            if (i + 1 < argc) {
                Filter filter;
                filter.type = Filter::Type::NAME;
                filter.namePattern = argv[++i];
                filter.include = true;
                options.filters.push_back(filter);
            } else {
                throw std::runtime_error("Опция -n требует шаблона");
            }
        } else if (arg == "-x" || arg == "--exclude") {
            if (i + 1 < argc) {
                Filter filter;
                filter.type = Filter::Type::NAME;
                filter.namePattern = argv[++i];
                filter.include = false;
                options.filters.push_back(filter);
            } else {
                throw std::runtime_error("Опция -x требует шаблона");
            }
        } else if (arg[0] != '-') {
            options.path = arg;
        }
    }
    
    return options;
}

std::unique_ptr<ITreeBuilder> CommandLineParser::createBuilder(const CommandLineOptions& options) {
    if (!options.filters.empty()) {
        auto builder = std::make_unique<FilteredTreeBuilder>(options.path);
        if (options.maxDepth > 0) {
            dynamic_cast<FilteredTreeBuilder*>(builder.get())->setMaxDepth(options.maxDepth);
        }
        applyFilters(builder.get(), options.filters);
        return builder;
    } else if (options.maxDepth > 0) {
        return std::make_unique<DepthViewTreeBuilder>(options.path, options.maxDepth);
    } else {
        return ITreeBuilder::create(options.path);
    }
}

void CommandLineParser::applyFilters(ITreeBuilder* builder, const std::vector<Filter>& filters) {
    FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder);
    if (!filteredBuilder) return;
    
    for (const auto& filter : filters) {
        switch (filter.type) {
            case Filter::Type::SIZE:
                filteredBuilder->addSizeFilter(filter.sizeValue, filter.operation);
                break;
            case Filter::Type::DATE:
                filteredBuilder->addDateFilter(filter.dateValue, filter.operation);
                break;
            case Filter::Type::NAME:
                filteredBuilder->addNameFilter(filter.namePattern, filter.include);
                break;
        }
    }
}