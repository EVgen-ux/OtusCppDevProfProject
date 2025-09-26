#include "TreeBuilderFactory.h"

std::unique_ptr<TreeBuilder> TreeBuilderFactory::createBuilder(
    const std::string& path, 
    bool useJSON,
    size_t maxDepth,
    bool useFilters) {
    
    std::unique_ptr<TreeBuilder> builder;
    
    if (useFilters) {
        builder = std::make_unique<FilteredTreeBuilder>(path);
    } else if (maxDepth > 0) {
        builder = std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
    } else {
        builder = std::make_unique<TreeBuilder>(path);
    }
    
    if (useJSON) {
        // Для JSON создаем специальный builder
        return std::make_unique<JSONTreeBuilder>(path);
    }
    
    return builder;
}