#include <iostream>
#include <memory>
#include "CommandLineParser.h"
#include "BuilderFactory.h"
#include "OutputManager.h"

int main(int argc, char* argv[]) {
    CommandLineOptions options;
    std::unique_ptr<TreeBuilder> builder;
    
    // Парсинг аргументов командной строки
    if (!CommandLineParser::parser(argc, argv, options, builder)) {
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
    
    CommandLineParser::applyFilters(options, *builder);

    try {
        builder->buildTree(options.showHidden);
        
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