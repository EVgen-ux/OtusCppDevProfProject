#include "HelpPrinter.h"
#include "constants.h"
#include <iostream>

void HelpPrinter::printHelp() {
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
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  tree-utility . -L 2           # Показать дерево глубиной 2 уровня" << std::endl;
    std::cout << "  tree-utility /path/to/dir -a -L 3" << std::endl;
    std::cout << "  tree-utility . -s \"> 100MB\"   # Файлы > 100MB" << std::endl;
    std::cout << "  tree-utility . -d \"> 2023-01-01\" # Файлы после 2023-01-01" << std::endl;
    std::cout << "  tree-utility . -n \"*.cpp\"      # Только .cpp файлы" << std::endl;
    std::cout << "  tree-utility . -x \"test.*\"     # Исключить test файлы" << std::endl;
}

void HelpPrinter::printVersion() {
    std::cout << "Tree Utility v" << constants::VERSION << std::endl;
    std::cout << "Автор: " << constants::AUTHOR << std::endl;
}