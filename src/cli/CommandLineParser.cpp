#include "CommandLineParser.h"
#include "FilteredTreeBuilder.h"
#include <iostream>
#include <algorithm>

bool CommandLineParser::parse(int argc, char* argv[], CommandLineOptions& options, std::unique_ptr<TreeBuilder>& builder) {
    options.useFilteredBuilder = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
            return false;
        } else if (arg == "-v" || arg == "--version") {
            options.showVersion = true;
            return false;
        } else if (arg == "-a" || arg == "--all") {
            options.showHidden = true;
        } else if (arg == "--no-color") {
            // Обработка отключения цветов
        } else if (arg == "--json") {
            options.useJSON = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                options.outputFile = argv[++i];
            } else {
                std::cerr << "Ошибка: отсутствует имя файла для опции -o" << std::endl;
                return false;
            }
        } else if (arg == "-L" || arg == "--level") {
            if (i + 1 < argc) {
                try {
                    options.maxDepth = std::stoul(argv[++i]);
                } catch (...) {
                    std::cerr << "Ошибка: неверный формат глубины" << std::endl;
                    return false;
                }
            }
        } else if (arg == "-s" || arg == "--size") {
            if (i + 1 < argc) {
                options.sizeFilter = argv[++i];
                options.useFilteredBuilder = true;
            }
        } else if (arg == "-d" || arg == "--date") {
            if (i + 1 < argc) {
                options.dateFilter = argv[++i];
                options.useFilteredBuilder = true;
            }
        } else if (arg == "-n" || arg == "--name") {
            if (i + 1 < argc) {
                options.nameFilter = argv[++i];
                options.useFilteredBuilder = true;
            }
        } else if (arg == "-x" || arg == "--exclude") {
            if (i + 1 < argc) {
                options.excludeFilter = argv[++i];
                options.useFilteredBuilder = true;
            }
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                std::string threadArg = argv[++i];
                if (threadArg == "auto") {
                    options.threadCount = 0;
                } else {
                    try {
                        options.threadCount = std::stoul(threadArg);
                    } catch (...) {
                        std::cerr << "Ошибка: неверный формат количества потоков" << std::endl;
                        return false;
                    }
                }
            }
        } else if (arg == "-g" || arg == "--github") {
            if (i + 1 < argc) {
                options.githubUrl = argv[++i];
                options.isGitHub = true;
            }
        } else if (arg == "--github-depth") {
            if (i + 1 < argc) {
                try {
                    options.githubDepth = std::stoul(argv[++i]);
                } catch (...) {
                    std::cerr << "Ошибка: неверный формат глубины GitHub" << std::endl;
                    return false;
                }
            }
        } else {
            // Если это не опция, то это путь
            if (arg[0] != '-') {
                options.path = arg;
            } else {
                std::cerr << "Ошибка: неизвестная опция " << arg << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

uint64_t CommandLineParser::parseSize(const std::string& sizeStr) {
    if (sizeStr.empty()) return 0;
    
    std::string valueStr = sizeStr;
    std::string unit;
    
    // Убираем пробелы в начале и конце
    valueStr.erase(0, valueStr.find_first_not_of(" "));
    valueStr.erase(valueStr.find_last_not_of(" ") + 1);
    
    // Ищем начало единицы измерения (первую не-цифру и не точку)
    size_t unitStart = 0;
    while (unitStart < valueStr.length() && 
           (std::isdigit(valueStr[unitStart]) || valueStr[unitStart] == '.')) {
        unitStart++;
    }
    
    if (unitStart < valueStr.length()) {
        unit = valueStr.substr(unitStart);
        valueStr = valueStr.substr(0, unitStart);
        
        // Убираем пробелы из единицы измерения
        unit.erase(0, unit.find_first_not_of(" "));
        unit.erase(unit.find_last_not_of(" ") + 1);
    }
    
    try {
        double value = std::stod(valueStr);
        
        // Приводим к байтам
        if (unit.empty() || unit == "B") return static_cast<uint64_t>(value);
        else if (unit == "KB") return static_cast<uint64_t>(value * 1000);
        else if (unit == "MB") return static_cast<uint64_t>(value * 1000 * 1000);
        else if (unit == "GB") return static_cast<uint64_t>(value * 1000 * 1000 * 1000);
        else if (unit == "TB") return static_cast<uint64_t>(value * 1000 * 1000 * 1000 * 1000);
        else if (unit == "KiB") return static_cast<uint64_t>(value * 1024);
        else if (unit == "MiB") return static_cast<uint64_t>(value * 1024 * 1024);
        else if (unit == "GiB") return static_cast<uint64_t>(value * 1024 * 1024 * 1024);
        else if (unit == "TiB") return static_cast<uint64_t>(value * 1024 * 1024 * 1024 * 1024);
        
        // Если единица не распознана, предполагаем байты
        return static_cast<uint64_t>(value);
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга размера '" << sizeStr << "': " << e.what() << std::endl;
    }
    
    return 0;
}

void CommandLineParser::applyFilters(CommandLineOptions& options, TreeBuilder& builder) {
    if (!options.useFilteredBuilder) return;
    
    if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(&builder)) {
        // Фильтр по размеру
        if (!options.sizeFilter.empty()) {
            std::string sizeStr = options.sizeFilter;
            std::string operation = ">";
            
            // Парсим операцию - ищем сначала длинные операторы
            if (sizeStr.find(">=") == 0) {
                operation = ">=";
                sizeStr = sizeStr.substr(2);
            } else if (sizeStr.find("<=") == 0) {
                operation = "<=";
                sizeStr = sizeStr.substr(2);
            } else if (sizeStr.find("==") == 0) {
                operation = "==";
                sizeStr = sizeStr.substr(2);
            } else if (sizeStr.find("!=") == 0) {
                operation = "!=";
                sizeStr = sizeStr.substr(2);
            } else if (sizeStr.find(">") == 0) {
                operation = ">";
                sizeStr = sizeStr.substr(1);
            } else if (sizeStr.find("<") == 0) {
                operation = "<";
                sizeStr = sizeStr.substr(1);
            }
            
            // Убираем пробелы
            sizeStr.erase(0, sizeStr.find_first_not_of(" "));
            sizeStr.erase(sizeStr.find_last_not_of(" ") + 1);
            
            uint64_t size = parseSize(sizeStr);
            if (size > 0) {
                filteredBuilder->addSizeFilter(size, operation);
                std::cout << "Применен фильтр размера: " << operation << " " 
                          << FileSystem::formatSize(size) << std::endl;
            } else {
                std::cerr << "Ошибка: не удалось распознать размер '" << sizeStr << "'" << std::endl;
            }
        }
        
        // Фильтр по имени (включение)
        if (!options.nameFilter.empty()) {
            filteredBuilder->addNameFilter(options.nameFilter, true);
        }
        
        // Фильтр по имени (исключение)
        if (!options.excludeFilter.empty()) {
            filteredBuilder->addNameFilter(options.excludeFilter, false);
        }
    }
}