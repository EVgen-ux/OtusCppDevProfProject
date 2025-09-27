#pragma once
#include <string>
#include <memory>
#include "TreeBuilder.h"

struct CommandLineOptions {
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
};

class CommandLineParser {
public:
    static bool parse(int argc, char* argv[], CommandLineOptions& options, std::unique_ptr<TreeBuilder>& builder);
    static uint64_t parseSize(const std::string& sizeStr);
    
private:
    static bool handleSizeFilter(const std::string& sizeArg, CommandLineOptions& options, 
                                std::unique_ptr<TreeBuilder>& builder, const std::string& path);
    static bool handleDateFilter(const std::string& dateArg, CommandLineOptions& options,
                                std::unique_ptr<TreeBuilder>& builder, const std::string& path);
    static bool handleNameFilter(const std::string& pattern, CommandLineOptions& options,
                                std::unique_ptr<TreeBuilder>& builder, const std::string& path, bool include);
};