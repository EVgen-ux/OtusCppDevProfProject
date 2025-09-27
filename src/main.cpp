#include <iostream>
#include <memory>
#include "CommandLineParser.h"
#include "BuilderFactory.h"
#include "OutputManager.h"

int main(int argc, char* argv[]) {
    CommandLineOptions options;
    std::unique_ptr<TreeBuilder> builder;
    
    // Парсинг аргументов командной строки
    if (!CommandLineParser::parse(argc, argv, options, builder)) {
        // Если вернулось false, значит была показана справка или версия
        if (argc > 1) {
            std::string firstArg = argv[1];
            if (firstArg == "-h" || firstArg == "--help") {
                OutputManager::printHelp();
            } else if (firstArg == "-v" || firstArg == "--version") {
                OutputManager::printVersion();
            }
        }
        return 0;
    }
    
    builder = BuilderFactory::create(options);

    try {
        // Добавляем информацию о потоках
        OutputManager::printThreadInfo(options.threadCount);

        // Строим дерево
        builder->buildTree(options.showHidden);
        
        // Вывод в файл или на консоль
        if (!options.outputFile.empty()) {
            if (!OutputManager::outputToFile(options.outputFile, *builder, options)) {
                return 1;
            }
        } else {
            OutputManager::outputToConsole(*builder, options);
        }


        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
