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
    
    // Убираем пробелы
    valueStr.erase(0, valueStr.find_first_not_of(" "));
    valueStr.erase(valueStr.find_last_not_of(" ") + 1);
    
    // Ищем конец числа (цифры и точка)
    size_t i = 0;
    while (i < valueStr.length() && 
           (std::isdigit(valueStr[i]) || valueStr[i] == '.')) {
        i++;
    }
    
    // Разделяем число и единицу измерения
    if (i < valueStr.length()) {
        valueStr = sizeStr.substr(0, i);
        unit = sizeStr.substr(i);
        
        // Убираем пробелы из unit
        unit.erase(0, unit.find_first_not_of(" "));
        unit.erase(unit.find_last_not_of(" ") + 1);
    }
    
    try {
        double value = std::stod(valueStr);
        
        // Приводим к верхнему регистру для удобства сравнения
        std::string upperUnit = unit;
        std::transform(upperUnit.begin(), upperUnit.end(), upperUnit.begin(), ::toupper);
        
        // Конвертируем в байты
        if (upperUnit.empty() || upperUnit == "B") {
            return static_cast<uint64_t>(value);
        } else if (upperUnit == "KB") {
            return static_cast<uint64_t>(value * 1024);
        } else if (upperUnit == "MB") {
            return static_cast<uint64_t>(value * 1024 * 1024);
        } else if (upperUnit == "GB") {
            return static_cast<uint64_t>(value * 1024 * 1024 * 1024);
        } else if (upperUnit == "TB") {
            return static_cast<uint64_t>(value * 1024 * 1024 * 1024 * 1024);
        // } else if (upperUnit == "KIB") {
        //     return static_cast<uint64_t>(value * 1000);
        // } else if (upperUnit == "MIB") {
        //     return static_cast<uint64_t>(value * 1000 * 1000);
        // } else if (upperUnit == "GIB") {
        //     return static_cast<uint64_t>(value * 1000 * 1000 * 1000);
        // } else if (upperUnit == "TIB") {
        //     return static_cast<uint64_t>(value * 1000 * 1000 * 1000 * 1000);
        } else {
            // Если единица не распознана, предполагаем байты
            return static_cast<uint64_t>(value);
        }
        
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
            
            // Парсим операцию
            if (sizeStr.size() >= 2) {
                std::string op2 = sizeStr.substr(0, 2);
                if (op2 == ">=" || op2 == "<=" || op2 == "==" || op2 == "!=") {
                    operation = op2;
                    sizeStr = sizeStr.substr(2);
                }
            }
            
            if (sizeStr.size() >= 1 && (operation == ">" || operation == "<")) {
                std::string op1 = sizeStr.substr(0, 1);
                if (op1 == ">" || op1 == "<") {
                    operation = op1;
                    sizeStr = sizeStr.substr(1);
                }
            }
            
            // Убираем пробелы
            sizeStr.erase(0, sizeStr.find_first_not_of(" "));
            sizeStr.erase(sizeStr.find_last_not_of(" ") + 1);
            
            uint64_t size = parseSize(sizeStr);
            if (size > 0) {
                filteredBuilder->addSizeFilter(size, operation);
                std::cout << "Применен фильтр размера: " << operation << " " 
                          << FileSystem::formatSize(size) << std::endl;
            }
        }
        
        // Фильтр по дате
        if (!options.dateFilter.empty()) {
            std::string dateStr = options.dateFilter;
            std::string operation = ">";
            
            // Парсим операцию (аналогично размеру)
            if (dateStr.size() >= 2) {
                std::string op2 = dateStr.substr(0, 2);
                if (op2 == ">=" || op2 == "<=" || op2 == "==") {
                    operation = op2;
                    dateStr = dateStr.substr(2);
                }
            }
            
            if (dateStr.size() >= 1 && (operation == ">" || operation == "<")) {
                std::string op1 = dateStr.substr(0, 1);
                if (op1 == ">" || op1 == "<") {
                    operation = op1;
                    dateStr = dateStr.substr(1);
                }
            }
            
            // Убираем пробелы
            dateStr.erase(0, dateStr.find_first_not_of(" "));
            dateStr.erase(dateStr.find_last_not_of(" ") + 1);
            
            filteredBuilder->addDateFilter(dateStr, operation);
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