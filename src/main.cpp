#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include "TreeBuilder.h"
#include "DepthViewTreeBuilder.h"
#include "GitHubTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "JSONTreeBuilder.h"
#include "FileSystem.h"
#include "ColorManager.h"
#include "MultiThreadedTreeBuilder.h"

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
    std::cout << "  -g, --github URL    Построить дерево из GitHub репозитория" << std::endl;
    std::cout << "  --github-depth N    Глубина для GitHub (по умолчанию: 3)" << std::endl;
    std::cout << "  -o, --output FILE   Сохранить вывод в файл" << std::endl;
    std::cout << "  -t, --threads N     Количество потоков (auto, 1, 2, 4, ...)" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  tree-utility . -L 2           # Показать дерево глубиной 2 уровня" << std::endl;
    std::cout << "  tree-utility . -a -L 3" << std::endl;
    std::cout << "  tree-utility . -s \"> 100MB\"   # Файлы > 100MB" << std::endl;
    std::cout << "  tree-utility . -d \"> 2023-01-01\" # Файлы после 2023-01-01" << std::endl;
    std::cout << "  tree-utility . -n \"*.cpp\"      # Только .cpp файлы" << std::endl;
    std::cout << "  tree-utility . -x \"test.*\"     # Исключить test файлы" << std::endl;
    std::cout << "  tree-utility . --json         # Вывод в формате JSON" << std::endl;
    std::cout << "  tree-utility . --json -o output.json # Сохранить в JSON файл" << std::endl;
    std::cout << "  tree-utility . -t auto        # Автоматическое определение потоков" << std::endl;
    std::cout << "  tree-utility . -t 4           # Использовать 4 потока" << std::endl;
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
    bool isGitHub = false;
    size_t githubDepth = 3;
    std::string outputFile;
    size_t threadCount = 1; 

    std::unique_ptr<TreeBuilder> builder;
    
    // Парсинг аргументов командной строки
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
            useColors = false;
            ColorManager::disableColors();
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
                useColors = false;
                ColorManager::disableColors();
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
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка: неверное значение для глубины: " << argv[i] << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Ошибка: опция -L требует значения" << std::endl;
                return 1;
            }
        } 
        else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                std::string threadArg = argv[++i];
                if (threadArg == "auto") {
                    threadCount = 0; // 0 означает auto
                } else {
                    try {
                        threadCount = std::stoul(threadArg);
                        if (threadCount == 0) {
                            std::cerr << "Ошибка: количество потоков должно быть > 0 или 'auto'" << std::endl;
                            return 1;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка: неверное значение для потоков: " << threadArg << std::endl;
                        return 1;
                    }
                }
            } else {
                std::cerr << "Ошибка: опция -t требует значения" << std::endl;
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
                    if (!builder) {
                        builder = std::make_unique<FilteredTreeBuilder>(path);
                    } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                        // Если builder уже создан, но не FilteredTreeBuilder, пересоздаем
                        builder = std::make_unique<FilteredTreeBuilder>(path);
                    }
                    
                    if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                        filteredBuilder->addSizeFilter(size, op);
                    }
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
                
                if (!builder) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                }
                
                if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    filteredBuilder->addDateFilter(dateStr, op);
                }
            } else {
                std::cerr << "Ошибка: опция -d требует значения" << std::endl;
                return 1;
            }
        } else if (arg == "-n" || arg == "--name") {
            useFilteredBuilder = true;
            if (i + 1 < argc) {
                std::string pattern = argv[++i];
                if (!builder) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                }
                
                if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    filteredBuilder->addNameFilter(pattern, true);
                }
            } else {
                std::cerr << "Ошибка: опция -n требует шаблона" << std::endl;
                return 1;
            }
        } else if (arg == "-x" || arg == "--exclude") {
            useFilteredBuilder = true;
            if (i + 1 < argc) {
                std::string pattern = argv[++i];
                if (!builder) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                } else if (!dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    builder = std::make_unique<FilteredTreeBuilder>(path);
                }
                
                if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
                    filteredBuilder->addNameFilter(pattern, false);
                }
            } else {
                std::cerr << "Ошибка: опция -x требует шаблона" << std::endl;
                return 1;
            }
        } else if (arg == "--github" || arg == "-g") {
    if (i + 1 < argc) {
        std::string githubUrl = argv[++i];
        builder = std::make_unique<GitHubTreeBuilder>(githubUrl, githubDepth);
        isGitHub = true;
        useFilteredBuilder = false;
    } else {
        std::cerr << "Ошибка: опция --github требует URL" << std::endl;
        return 1;
    }
} else if (arg == "--github-depth") {
    if (i + 1 < argc) {
        githubDepth = std::stoul(argv[++i]);
    }
}
        
        
        else if (arg[0] != '-') {
            path = arg;
        }
    }
 
    
 // Создаем builder
if (!builder) {
    if (useJSON) {
        builder = std::make_unique<JSONTreeBuilder>(path);
    } else if (maxDepth > 0) {
        builder = std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
    } else if (useFilteredBuilder) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    } else {
        // Проверяем многопоточность (кроме GitHub)
        if ((threadCount > 1 || threadCount == 0) && 
            !(path.find("github.com") != std::string::npos || 
              path.find("https://github.com") != std::string::npos)) {
            builder = std::make_unique<MultiThreadedTreeBuilder>(path, threadCount);
        } else {
            builder = std::make_unique<TreeBuilder>(path);
        }
    }
} else if (useJSON && !dynamic_cast<JSONTreeBuilder*>(builder.get())) {
    // Если запрошен JSON, но builder не JSONTreeBuilder, пересоздаем
    builder = std::make_unique<JSONTreeBuilder>(path);
}

// Применяем максимальную глубину если нужно
if (maxDepth > 0 && !useJSON) {
    if (auto depthBuilder = dynamic_cast<DepthViewTreeBuilder*>(builder.get())) {
        depthBuilder->setMaxDepth(maxDepth);
    } else if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(builder.get())) {
        filteredBuilder->setMaxDepth(maxDepth);
    }
}   


    try {
        // Строим дерево
        builder->buildTree(showHidden);
        // Вывод в файл или на консоль
        if (!outputFile.empty()) {
            std::ofstream outFile(outputFile);
            if (!outFile) {
                std::cerr << "Ошибка: не удалось открыть файл " << outputFile << " для записи" << std::endl;
                return 1;
            }

            if (isGitHub) {
                auto displayStats = builder->getDisplayStatistics();
                std::cout << "  Директорий: " << displayStats.displayedDirectories << std::endl;
                std::cout << "  Файлов: " << displayStats.displayedFiles << std::endl;
                std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
                std::cout << "  API запросов: " << displayStats.apiRequests << std::endl;
        
                if (displayStats.apiRequests >= 50) {
                std::cout << "  Близко к лимиту GitHub API (60/час)" << std::endl;
                }
            }
            
            if (useJSON) {
                // Для JSON выводим специальным методом
                if (auto jsonBuilder = dynamic_cast<JSONTreeBuilder*>(builder.get())) {
                    outFile << jsonBuilder->getJSON() << std::endl;
                } else {
                    std::cerr << "Ошибка: JSON builder не доступен" << std::endl;
                    return 1;
                }
            } else {
                const auto& treeLines = builder->getTreeLines();
                for (const auto& line : treeLines) {
                    outFile << line << std::endl;
                }
                
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
                
                auto displayStats = builder->getDisplayStatistics();
                if (displayStats.hiddenObjects > 0 && !showHidden) {
                    outFile << "  В каталоге есть скрытые объекты: " << displayStats.hiddenObjects 
                            << " (используйте -a для показа)" << std::endl;
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
            
            // Для JSON не выводим статистику (она уже в JSON)
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

                auto displayStats = builder->getDisplayStatistics();
                if (displayStats.hiddenObjects > 0 && !showHidden) {
                    std::cout << "  В каталоге есть скрытые объекты: " << displayStats.hiddenObjects 
                              << " (используйте -a для показа)" << std::endl;
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