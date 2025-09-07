#include <iostream>
#include <string>
#include "DepthViewTreeBuilder.h"
#include "constants.h"
#include "FileSystem.h"

void printHelp() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Использование: tree-utility [ПУТЬ] [ОПЦИИ]" << std::endl;
    std::cout << std::endl;
    std::cout << "Опции:" << std::endl;
    std::cout << "  -h, --help          Показать эту справку" << std::endl;
    std::cout << "  -a, --all           Показать скрытые файлы и папки" << std::endl;
    std::cout << "  -v, --version       Показать версию" << std::endl;
    std::cout << "  -L, --level N       Ограничить глубину дерева N уровнями" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  tree-utility . -L 2           # Показать дерево глубиной 2 уровня" << std::endl;
    std::cout << "  tree-utility /path/to/dir -a -L 3" << std::endl;
}

void printVersion() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Автор: " << constants::AUTHOR << std::endl;
}

int main(int argc, char* argv[]) {
    std::string path = ".";
    bool showHidden = false;
    size_t maxDepth = 0;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
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
        } else if (arg[0] != '-') {
            path = arg;
        }
    }
    
    try {
    DepthViewTreeBuilder builder(path, maxDepth);
    builder.buildTree(showHidden);
    builder.printTree();
    
    std::cout << std::endl;
    
    if (maxDepth > 0) {
        // Для ограниченной глубины используем displayStats
        auto displayStats = builder.getDisplayStatistics();
        std::cout << "Статистика (глубина ограничена " << maxDepth << " уровнями):" << std::endl;
        std::cout << "  Директорий: " << displayStats.displayedDirectories << std::endl;
        std::cout << "  Файлов: " << displayStats.displayedFiles << std::endl;
        std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
        if (displayStats.hiddenByDepth > 0) {
            std::cout << "  Скрыто по глубине: " << displayStats.hiddenByDepth << " директорий" << std::endl;
        }
    } else {
        // Для полного обхода используем общую статистику
        auto stats = builder.getStatistics();
        std::cout << "Статистика:" << std::endl;
        std::cout << "  Директорий: " << stats.totalDirectories << std::endl;
        std::cout << "  Файлов: " << stats.totalFiles << std::endl;
        std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(stats.totalSize) << std::endl;
    }
    
} catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
    return 1;
}
    
    return 0;
}