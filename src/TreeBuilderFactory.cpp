#include "TreeBuilderFactory.h"
#include "MultiThreadedTreeBuilder.h"

std::unique_ptr<TreeBuilder> TreeBuilderFactory::createBuilder(
    const std::string& path, 
    bool useJSON,
    size_t maxDepth,
    bool useFilters,
    size_t threadCount) {
    
    std::unique_ptr<TreeBuilder> builder;
    
    // Для GitHub всегда используем один поток
    if (path.find("github.com") != std::string::npos || 
        path.find("https://github.com") != std::string::npos) {
        if (useJSON) {
            return std::make_unique<JSONTreeBuilder>(path);
        } else if (maxDepth > 0) {
            return std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
        } else {
            return std::make_unique<TreeBuilder>(path);
        }
    }
    
    // Многопоточный builder
    if (threadCount > 1 || threadCount == 0) { // 0 = auto
        builder = std::make_unique<MultiThreadedTreeBuilder>(path, threadCount);
    } 
    // Остальные случаи
    else if (useFilters) {
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