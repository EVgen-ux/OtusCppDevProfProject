#include "BuilderFactory.h"
#include "JSONTreeBuilder.h"
#include "DepthViewTreeBuilder.h"
#include "FilteredTreeBuilder.h"
#include "MultiThreadedTreeBuilder.h"
#include "GitHubTreeBuilder.h"

std::unique_ptr<TreeBuilder> BuilderFactory::createBuilder(
    const std::string& path, 
    bool useJSON,
    size_t maxDepth,
    bool useFilters,
    size_t threadCount) {
    
    if (useJSON) {
        return std::make_unique<JSONTreeBuilder>(path);
    }
    
    // ИЕсли есть фильтры ИЛИ ограничение глубины, создаем FilteredTreeBuilder
    if (useFilters || maxDepth > 0) {
        auto builder = std::make_unique<FilteredTreeBuilder>(path);
        if (maxDepth > 0) {
            builder->setMaxDepth(maxDepth);
        }
        return builder;
    }
    
    // Многопоточность только для локальных путей (не GitHub)
    if ((threadCount > 1 || threadCount == 0) && 
        path.find("github.com") == std::string::npos) {
        return std::make_unique<MultiThreadedTreeBuilder>(path, threadCount);
    }
    
    return std::make_unique<TreeBuilder>(path);
}

std::unique_ptr<TreeBuilder> BuilderFactory::create(const CommandLineOptions& options) {
    return createBuilder(options.path, 
                        options.useJSON,
                        options.maxDepth,
                        options.useFilteredBuilder,
                        options.threadCount);
}

void BuilderFactory::applySettings(const CommandLineOptions& options, TreeBuilder& builder) {
    // Установка глубины для различных типов билдеров
    if (options.maxDepth > 0) {
        if (auto depthBuilder = dynamic_cast<DepthViewTreeBuilder*>(&builder)) {
            depthBuilder->setMaxDepth(options.maxDepth);
        } else if (auto filteredBuilder = dynamic_cast<FilteredTreeBuilder*>(&builder)) {
            filteredBuilder->setMaxDepth(options.maxDepth);
        } else if (auto githubBuilder = dynamic_cast<GitHubTreeBuilder*>(&builder)) {
            githubBuilder->setMaxDepth(options.maxDepth);
        }
    }
}