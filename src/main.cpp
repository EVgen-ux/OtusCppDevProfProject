#include <iostream>
#include <string>
#include "DepthViewTreeBuilder.h"
#include "constants.h"

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
        
        auto stats = builder.getStatistics();
        std::cout << std::endl;
        if (maxDepth > 0) {
            std::cout << "Статистика (глубина ограничена " << maxDepth << " уровнями):" << std::endl;
        } else {
            std::cout << "Статистика:" << std::endl;
        }
        std::cout << "  Директорий: " << stats.totalDirectories << std::endl;
        std::cout << "  Файлов: " << stats.totalFiles << std::endl;
        std::cout << "  Общий размер: " << stats.totalSize << " байт" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}