#pragma once
#include <iostream>
#include <fstream>
#include <memory>
#include "TreeBuilder.h"
#include "CommandLineParser.h"

class OutputManager {
public:
    static void printHelp();
    static void printVersion();
    static void printStatistics(std::ostream& output, const TreeBuilder& builder, 
                               const CommandLineOptions& options);
    static void printThreadInfo(size_t threadCount);
    
    static bool outputToFile(const std::string& filename, const TreeBuilder& builder, 
                            const CommandLineOptions& options);
    static void outputToConsole(const TreeBuilder& builder, const CommandLineOptions& options);
};