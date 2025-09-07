#include <iostream>
#include <memory>
#include "CommandLineParser.h"
#include "HelpPrinter.h"
#include "StatisticsPrinter.h"

int main(int argc, char* argv[]) {
    // Проверка аргументов помощи/версии
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            HelpPrinter::printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            HelpPrinter::printVersion();
            return 0;
        }
    }
    
    try {
        // Парсинг командной строки
        CommandLineOptions options = CommandLineParser::parse(argc, argv);
        
        // Создание билдера
        std::unique_ptr<ITreeBuilder> builder = CommandLineParser::createBuilder(options);
        
        // Вывод информации об ограничении глубины
        if (options.maxDepth > 0) {
            std::cout << "Глубина ограничена " << options.maxDepth << " уровнями" << std::endl;
        }
        
        // Вывод информации о фильтрах
        if (!options.filters.empty()) {
            std::cout << "Применены фильтры: " << options.filters.size() << std::endl;
        }
        
        // Построение и вывод дерева
        builder->buildTree(options.showHidden);
        builder->printTree();
        
        // Вывод статистики
        StatisticsPrinter::printStatistics(*builder, !options.filters.empty());
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}