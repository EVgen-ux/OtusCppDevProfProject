#include <iostream>
#include <string>
#include <memory>
#include <fstream> 
#include "DepthViewTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "JSONTreeBuilder.h"
#include "constants.h"
#include "FileSystem.h"
#include "ColorManager.h" // Добавьте этот include

void printHelp() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Использование: tree-utility [ПУТЬ] [ОПЦИИ]" << std::endl;
    std::cout << std::endl;
    std::cout << "Опции:" << std::endl;
    std::cout << "  -h, --help          Показать эту справку" << std::endl;
    std::cout << "  -a, --all           Показать скрытые файлы и папки" << std::endl;
    std::cout << "  -v, --version       Показать версию" << std::endl;
    std::cout << "  -L, --level N       Ограничить глубину дерева N уровнями" << std::endl;
    std::cout << "  -s, --size OP SIZE  Фильтр по размеру (>, <, ==, >=, <=)" << std::endl;
    std::cout << "  -d, --date OP DATE  Фильтр по дате (>, <, ==), формат: YYYY-MM-DD" << std::endl;
    std::cout << "  -n, --name PATTERN  Включить файлы по шаблону имени" << std::endl;
    std::cout << "  -x, --exclude PATTERN Исключить файлы по шаблону имени" << std::endl;
    std::cout << "  --no-color          Отключить цветное оформление" << std::endl;
    std::cout << "  --json              Вывод в формате JSON" << std::endl; 
    std::cout << "  -o, --output FILE   Сохранить вывод в файл" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  tree-utility . -L 2           # Показать дерево глубиной 2 уровня" << std::endl;
    std::cout << "  tree-utility /path/to/dir -a -L 3" << std::endl;
    std::cout << "  tree-utility . -s \"> 100MB\"   # Файлы > 100MB" << std::endl;
    std::cout << "  tree-utility . -d \"> 2023-01-01\" # Файлы после 2023-01-01" << std::endl;
    std::cout << "  tree-utility . -n \"*.cpp\"      # Только .cpp файлы" << std::endl;
    std::cout << "  tree-utility . -x \"test.*\"     # Исключить test файлы" << std::endl;
    std::cout << std::endl;
    std::cout << "Примечание: порядок опций влияет на результат обработки:" << std::endl;
    std::cout << "  -L 2 -s \">100MB\"  # Сначала ограничить глубину, потом фильтровать" << std::endl;
    std::cout << "  -s \">100MB\" -L 2  # Сначала отфильтровать, потом ограничить глубину" << std::endl;
    std::cout << "  Можно комбинировать несколько фильтров: -s '>100MB' -x '*.tmp'" << std::endl;
}

void printVersion() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Автор: " << constants::AUTHOR << std::endl;
}

uint64_t parseSize(const std::string& sizeStr) {
    size_t multiplier = 1;
    std::string numStr = sizeStr;
    
    // Удаляем возможные пробелы
    numStr.erase(std::remove_if(numStr.begin(), numStr.end(), ::isspace), numStr.end());
    
    // Проверяем суффиксы (case-insensitive)
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
    
    // Удаляем оставшиеся пробелы из числовой части
    numStr.erase(std::remove_if(numStr.begin(), numStr.end(), ::isspace), numStr.end());
    
    try {
        double size = std::stod(numStr);
        return static_cast<uint64_t>(size * multiplier);
    } catch (const std::exception&) {
        throw std::runtime_error("Неверный формат размера: " + sizeStr);
    }
}

int main(int argc, char* argv[]) {
    std::string path = ".";
    bool showHidden = false;
    size_t maxDepth = 0;
    bool useFilteredBuilder = false;
    bool useColors = true;
    bool useJSON = false;
    std::string outputFile;

    std::unique_ptr<ITreeBuilder> builder;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "--no-color") {
            useColors = false;
            ColorManager::disableColors();
        } else if (arg == "--json") {
            useJSON = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Ошибка: опция -o требует имени файла" << std::endl;
                return 1;
            }
        } else if (arg == "-a" || arg == "--all") {
            showHidden = true;
        } else if (arg == "-L" || arg == "--level") {
            if (i + 1 < argc) {
                try {
                    maxDepth = std::stoul(argv[++i]);
                    // Пересоздаем билдер с новой глубиной
                    if (maxDepth > 0) {
                        builder = std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
                    } else {
                        builder = ITreeBuilder::create(path);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка: неверное значение для глубины: " << argv[i] << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Ошибка: опция -L требует значения" << std::endl;
                return 1;
            }
        } else if (arg == "-s" || arg == "--size") {
            useFilteredBuilder = true;
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
                        std::cerr << "Ошибка: неверный формат размера: " << sizeArg << std::endl;
                        return 1;
                    }
                }
                
                try {
                    uint64_t size = parseSize(sizeStr);
                    FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                    if (!filteredBuilder) {
                        size_t currentDepth = maxDepth;
                        builder = std::make_unique<FilteredTreeBuilder>(path);
                        filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                        if (currentDepth > 0) {
                            filteredBuilder->setMaxDepth(currentDepth);
                        }
                    }
                    filteredBuilder->addSizeFilter(size, op);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка: " << e.what() << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Ошибка: опция -s требует значения" << std::endl;
                return 1;
            }
        } else if (arg == "-d" || arg == "--date") {
            useFilteredBuilder = true;
            if (i + 1 < argc) {
                std::string dateArg = argv[++i];
                std::string op;
                std::string dateStr;
                
                size_t spacePos = dateArg.find(' ');
                if (spacePos != std::string::npos) {
                    op = dateArg.substr(0, spacePos);
                    dateStr = dateArg.substr(spacePos + 1);
                } else {
                    std::cerr << "Ошибка: неверный формат даты: " << dateArg << std::endl;
                    return 1;
                }
                
                FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                if (!filteredBuilder) {
                    size_t currentDepth = maxDepth;
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                    filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                    if (currentDepth > 0) {
                        filteredBuilder->setMaxDepth(currentDepth);
                    }
                }
                filteredBuilder->addDateFilter(dateStr, op);
            } else {
                std::cerr << "Ошибка: опция -d требует значения" << std::endl;
                return 1;
            }
        } else if (arg == "-n" || arg == "--name") {
            useFilteredBuilder = true;
            if (i + 1 < argc) {
                std::string pattern = argv[++i];
                FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                if (!filteredBuilder) {
                    size_t currentDepth = maxDepth;
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                    filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                    if (currentDepth > 0) {
                        filteredBuilder->setMaxDepth(currentDepth);
                    }
                }
                filteredBuilder->addNameFilter(pattern, true);
            } else {
                std::cerr << "Ошибка: опция -n требует шаблона" << std::endl;
                return 1;
            }
        } else if (arg == "-x" || arg == "--exclude") {
            useFilteredBuilder = true;
            if (i + 1 < argc) {
                std::string pattern = argv[++i];
                FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                if (!filteredBuilder) {
                    size_t currentDepth = maxDepth;
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                    filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                    if (currentDepth > 0) {
                        filteredBuilder->setMaxDepth(currentDepth);
                    }
                }
                filteredBuilder->addNameFilter(pattern, false);
            } else {
                std::cerr << "Ошибка: опция -x требует шаблона" << std::endl;
                return 1;
            }
        } else if (arg[0] != '-') {
            path = arg;
            // Обновляем путь в билдере
            if (DepthViewTreeBuilder* depthBuilder = dynamic_cast<DepthViewTreeBuilder*>(builder.get())) {
                builder = std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
            } else if (FilteredTreeBuilder* filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                builder = std::make_unique<FilteredTreeBuilder>(path);
                filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get());
                if (maxDepth > 0) {
                    filteredBuilder->setMaxDepth(maxDepth);
                }
            } else {
                builder = ITreeBuilder::create(path);
            }
        }
    }
    
    // Если билдер еще не создан, создаем его
    if (!builder) {
        if (maxDepth > 0) {
            builder = std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
        } else {
            builder = ITreeBuilder::create(path);
        }
    }

    // Обертываем в JSON builder если нужно
    if (useJSON) {
        builder = std::make_unique<JSONTreeBuilder>(std::move(builder));
    }

    try {
        builder->buildTree(showHidden);
        
        // Вывод в файл или на консоль
        if (!outputFile.empty()) {
            std::ofstream outFile(outputFile);
            if (!outFile) {
                std::cerr << "Ошибка: не удалось открыть файл " << outputFile << " для записи" << std::endl;
                return 1;
            }
            
            if (useJSON) {
                // Для JSON выводим напрямую
                auto jsonBuilder = dynamic_cast<JSONTreeBuilder*>(builder.get());
                if (jsonBuilder) {
                    outFile << jsonBuilder->getJSON() << std::endl;
                }
            } else {
                // Для обычного вывода сохраняем дерево
                const auto& treeLines = builder->getTreeLines();
                for (const auto& line : treeLines) {
                    outFile << line << std::endl;
                }
                
                // Сохраняем статистику
                outFile << std::endl;
                outFile << "Статистика:" << std::endl;
                
                if (maxDepth > 0) {
                    auto displayStats = builder->getDisplayStatistics();
                    outFile << "  Директорий: " << displayStats.displayedDirectories << std::endl;
                    outFile << "  Файлов: " << displayStats.displayedFiles << std::endl;
                    outFile << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
                    if (displayStats.hiddenByDepth > 0) {
                        outFile << "  Скрыто по глубине: " << displayStats.hiddenByDepth << " директорий" << std::endl;
                    }
                } else {
                    auto stats = builder->getStatistics();
                    outFile << "  Директорий: " << stats.totalDirectories << std::endl;
                    outFile << "  Файлов: " << stats.totalFiles << std::endl;
                    outFile << "  Общий размер: " << FileSystem::formatSizeBothSystems(stats.totalSize) << std::endl;
                }
                
                if (useFilteredBuilder) {
                    outFile << "  (Применены фильтры)" << std::endl;
                }
            }
            
            std::cout << "Результат сохранен в файл: " << outputFile << std::endl;
        } else {
            // Вывод на консоль
            if (maxDepth > 0 && !useJSON) {
                std::cout << "Глубина ограничена " << maxDepth << " уровнями" << std::endl;
            }
            
            builder->printTree();
            
            if (!useJSON) {
                std::cout << std::endl;
                std::cout << "Статистика:" << std::endl;
                
                if (maxDepth > 0) {
                    auto displayStats = builder->getDisplayStatistics();
                    std::cout << "  Директорий: " << displayStats.displayedDirectories << std::endl;
                    std::cout << "  Файлов: " << displayStats.displayedFiles << std::endl;
                    std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
                    if (displayStats.hiddenByDepth > 0) {
                        std::cout << "  Скрыто по глубине: " << displayStats.hiddenByDepth << " директорий" << std::endl;
                    }
                } else {
                    auto stats = builder->getStatistics();
                    std::cout << "  Директорий: " << stats.totalDirectories << std::endl;
                    std::cout << "  Файлов: " << stats.totalFiles << std::endl;
                    std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(stats.totalSize) << std::endl;
                }
                
                if (useFilteredBuilder) {
                    std::cout << "  (Применены фильтры)" << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}