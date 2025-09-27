#pragma once
#include "TreeBuilder.h"
#include "DepthViewTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "JSONTreeBuilder.h"
#include "MultiThreadedTreeBuilder.h"
#include <memory>

class TreeBuilderFactory {
public:
    static std::unique_ptr<TreeBuilder> createBuilder(
        const std::string& path, 
        bool useJSON = false,
        size_t maxDepth = 0,
        bool useFilters = false,
        size_t threadCount = 1 
    );
};