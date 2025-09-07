#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ITreeBuilder.h"

struct Filter {
    enum class Type { SIZE, DATE, NAME } type;
    std::string operation;
    uint64_t sizeValue = 0;
    std::string dateValue;
    std::string namePattern;
    bool include = true;
};

struct CommandLineOptions {
    std::string path = ".";
    bool showHidden = false;
    size_t maxDepth = 0;
    std::vector<Filter> filters;
};

class CommandLineParser {
public:
    static CommandLineOptions parse(int argc, char* argv[]);
    static std::unique_ptr<ITreeBuilder> createBuilder(const CommandLineOptions& options);
    static void applyFilters(ITreeBuilder* builder, const std::vector<Filter>& filters);
};