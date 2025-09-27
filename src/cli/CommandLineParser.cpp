#include "CommandLineParser.h"
#include "FilteredTreeBuilder.h"
#include "GitHubTreeBuilder.h"
#include "ColorManager.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>

bool CommandLineParser::parse(int argc, char* argv[], CommandLineOptions& options, std::unique_ptr<TreeBuilder>& builder) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            return false;
        } else if (arg == "-v" || arg == "--version") {
            return false;
        } else if (arg == "--no-color") {
            options.useColors = false;
            ColorManager::disableColors();
        } else if (arg == "--json") {
            options.useJSON = true;
            options.useColors = false;
            ColorManager::disableColors();
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                options.outputFile = argv[++i];
                options.useColors = false;
                ColorManager::disableColors();
            } else {
                std::cerr << "Ошибка: опция -o требует имени файла" << std::endl;
                return false;
            }
        } else if (arg == "-a" || arg == "--all") {
            options.showHidden = true;
        } else if (arg == "-L" || arg == "--level") {
            if (i + 1 < argc) {
                try {
                    options.maxDepth = std::stoul(argv[++i]);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка: неверное значение для глубины: " << argv[i] << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Ошибка: опция -L требует значения" << std::endl;
                return false;
            }
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                std::string threadArg = argv[++i];
                if (threadArg == "auto") {
                    options.threadCount = 0;
                } else {
                    try {
                        options.threadCount = std::stoul(threadArg);
                        if (options.threadCount == 0) {
                            std::cerr << "Ошибка: количество потоков должно быть > 0 или 'auto'" << std::endl;
                            return false;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка: неверное значение для потоков: " << threadArg << std::endl;
                        return false;
                    }
                }
            } else {
                std::cerr << "Ошибка: опция -t требует значения" << std::endl;
                return false;
            }
        } else if (arg == "-s" || arg == "--size") {
            options.useFilteredBuilder = true;
            if (i + 1 < argc) {
                if (!handleSizeFilter(argv[++i], options, builder, options.path)) {
                    return false;
                }
            } else {
                std::cerr << "Ошибка: опция -s требует значения" << std::endl;
                return false;
            }
        } else if (arg == "-d" || arg == "--date") {
            options.useFilteredBuilder = true;
            if (i + 1 < argc) {
                if (!handleDateFilter(argv[++i], options, builder, options.path)) {
                    return false;
                }
            } else {
                std::cerr << "Ошибка: опция -d требует значения" << std::endl;
                return false;
            }
        } else if (arg == "-n" || arg == "--name") {
            options.useFilteredBuilder = true;
            if (i + 1 < argc) {
                if (!handleNameFilter(argv[++i], options, builder, options.path, true)) {
                    return false;
                }
            } else {
                std::cerr << "Ошибка: опция -n требует шаблона" << std::endl;
                return false;
            }
        } else if (arg == "-x" || arg == "--exclude") {
            options.useFilteredBuilder = true;
            if (i + 1 < argc) {
                if (!handleNameFilter(argv[++i], options, builder, options.path, false)) {
                    return false;
                }
            } else {
                std::cerr << "Ошибка: опция -x требует шаблона" << std::endl;
                return false;
            }
        } else if (arg == "--github" || arg == "-g") {
            if (i + 1 < argc) {
                std::string githubUrl = argv[++i];
                builder = std::make_unique<GitHubTreeBuilder>(githubUrl, options.githubDepth);
                options.isGitHub = true;
                options.useFilteredBuilder = false;
            } else {
                std::cerr << "Ошибка: опция --github требует URL" << std::endl;
                return false;
            }
        } else if (arg == "--github-depth") {
            if (i + 1 < argc) {
                options.githubDepth = std::stoul(argv[++i]);
            }
        } else if (arg[0] != '-') {
            options.path = arg;
        }
    }
    
    return true;
}

uint64_t CommandLineParser::parseSize(const std::string& sizeStr) {
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

bool CommandLineParser::handleSizeFilter(const std::string& sizeArg, CommandLineOptions& options, 
                                       std::unique_ptr<TreeBuilder>& builder, const std::string& path) {
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
            std::cerr << "Ошибка: неверный формат размера: " << sizeArg << std::endl;
            return false;
        }
    }
    
    try {
        uint64_t size = parseSize(sizeStr);
        if (!builder) {
            builder = std::make_unique<FilteredTreeBuilder>(path);
        } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
            builder = std::make_unique<FilteredTreeBuilder>(path);
        }
        
        if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
            filteredBuilder->addSizeFilter(size, op);
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

bool CommandLineParser::handleDateFilter(const std::string& dateArg, CommandLineOptions& options,
                                       std::unique_ptr<TreeBuilder>& builder, const std::string& path) {
    std::string op;
    std::string dateStr;
    
    size_t spacePos = dateArg.find(' ');
    if (spacePos != std::string::npos) {
        op = dateArg.substr(0, spacePos);
        dateStr = dateArg.substr(spacePos + 1);
    } else {
        std::cerr << "Ошибка: неверный формат даты: " << dateArg << std::endl;
        return false;
    }
    
    if (!builder) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    }
    
    if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
        filteredBuilder->addDateFilter(dateStr, op);
    }
    
    return true;
}

bool CommandLineParser::handleNameFilter(const std::string& pattern, CommandLineOptions& options,
                                       std::unique_ptr<TreeBuilder>& builder, const std::string& path, bool include) {
    if (!builder) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    }
    
    if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
        filteredBuilder->addNameFilter(pattern, include);
    }
    
    return true;
}