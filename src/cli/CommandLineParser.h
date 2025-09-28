#pragma once
#include <string>
#include <memory>
#include "TreeBuilder.h"

struct CommandLineOptions {
    std::string path = ".";
    bool showHidden = false;
    bool showHelp = false;
    bool showVersion = false;
    bool noColor = false;
    size_t maxDepth = 0;
    bool useJSON = false;
    std::string outputFile;
    bool useFilteredBuilder = false;
    bool isGitHub = false;
    std::string githubUrl;
    size_t githubDepth = 3;
    size_t threadCount = 1;
    
    // Фильтры
    std::string sizeFilter;
    std::string dateFilter;
    std::string nameFilter;
    std::string excludeFilter;
};

class CommandLineParser {
public:
    static bool parse(int argc, char* argv[], CommandLineOptions& options, std::unique_ptr<TreeBuilder>& builder);
    static void applyFilters(CommandLineOptions& options, TreeBuilder& builder);
private:
    static uint64_t parseSize(const std::string& sizeStr);
};