#include "OutputManager.h"
#include "JSONTreeBuilder.h"
//#include "FileSystem.h"
//#include "Constants.h"
#include <iostream>

void OutputManager::printHelp() {
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

void OutputManager::printVersion() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Автор: " << constants::AUTHOR << std::endl;
}

void OutputManager::printStatistics(std::ostream& output, const TreeBuilder& builder, 
                                   const CommandLineOptions& options) {
    output << std::endl;
    output << "Статистика:" << std::endl;
    
    if (options.isGitHub) {
        auto displayStats = builder.getDisplayStatistics();
        output << "  Директорий: " << displayStats.displayedDirectories << std::endl;
        output << "  Файлов: " << displayStats.displayedFiles << std::endl;
        output << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
        output << "  API запросов: " << displayStats.apiRequests << std::endl;
        
        if (displayStats.apiRequests >= 50) {
            output << "  Близко к лимиту GitHub API (60/час)" << std::endl;
        }
    }
    else if (options.maxDepth > 0) {
        auto displayStats = builder.getDisplayStatistics();
        output << "  Директорий: " << displayStats.displayedDirectories << std::endl;
        output << "  Файлов: " << displayStats.displayedFiles << std::endl;
        output << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
        if (displayStats.hiddenByDepth > 0) {
            output << "  Скрыто по глубине: " << displayStats.hiddenByDepth << " директорий" << std::endl;
        }
    } else {
        auto stats = builder.getStatistics();
        output << "  Директорий: " << stats.totalDirectories << std::endl;
        output << "  Файлов: " << stats.totalFiles << std::endl;
        output << "  Общий размер: " << FileSystem::formatSizeBothSystems(stats.totalSize) << std::endl;
    }
    
    auto displayStats = builder.getDisplayStatistics();
    if (displayStats.hiddenObjects > 0 && !options.showHidden) {
        output << "  В каталоге есть скрытые объекты: " << displayStats.hiddenObjects 
               << " (используйте -a для показа)" << std::endl;
    }
    
    if (options.useFilteredBuilder) {
        output << "  (Применены фильтры)" << std::endl;
    }
}

void OutputManager::printThreadInfo(size_t threadCount) {
    if (threadCount > 1 || threadCount == 0) {
        std::cout << "Использовано потоков: " 
                  << (threadCount == 0 ? "auto" : std::to_string(threadCount)) 
                  << std::endl;
    }
}

bool OutputManager::outputToFile(const std::string& filename, const TreeBuilder& builder, 
                                const CommandLineOptions& options) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << " для записи" << std::endl;
        return false;
    }

    if (options.useJSON) {
        if (auto jsonBuilder = dynamic_cast<const JSONTreeBuilder*>(&builder)) {
            outFile << jsonBuilder->getJSON() << std::endl;
        } else {
            std::cerr << "Ошибка: JSON builder не доступен" << std::endl;
            return false;
        }
    } else {
        const auto& treeLines = builder.getTreeLines();
        for (const auto& line : treeLines) {
            outFile << line << std::endl;
        }
        
        if (!options.useJSON) {
            printStatistics(outFile, builder, options);
        }
    }
    
    std::cout << "Результат сохранен в файл: " << filename << std::endl;
    return true;
}

void OutputManager::outputToConsole(const TreeBuilder& builder, const CommandLineOptions& options) {
    if (options.maxDepth > 0 && !options.useJSON) {
        std::cout << "Глубина ограничена " << options.maxDepth << " уровнями" << std::endl;
    }
    
    builder.printTree();
    
    if (!options.useJSON) {
        printStatistics(std::cout, builder, options);
    }
}