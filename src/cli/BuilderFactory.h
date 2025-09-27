#pragma once
#include <memory>
#include "TreeBuilder.h"
#include "DepthViewTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "JSONTreeBuilder.h"
#include "MultiThreadedTreeBuilder.h"
#include "GitHubTreeBuilder.h"
#include "CommandLineParser.h"

class BuilderFactory {
public:
    static std::unique_ptr<TreeBuilder> create(const CommandLineOptions& options);
    static void applySettings(const CommandLineOptions& options, TreeBuilder& builder);
    
private:
    static std::unique_ptr<TreeBuilder> createBuilder(const std::string& path, 
                                                     bool useJSON = false,
                                                     size_t maxDepth = 0,
                                                     bool useFilters = false,
                                                     size_t threadCount = 1);
};