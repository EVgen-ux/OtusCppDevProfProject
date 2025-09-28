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
    
    // ИСПРАВЛЕНИЕ: проверяем GitHub URL перед другими условиями
    if (path.find("github.com") != std::string::npos) {
        return std::make_unique<GitHubTreeBuilder>(path, maxDepth);
    }
    
    // ПРИОРИТЕТ 1: Если есть фильтры - создаем FilteredTreeBuilder
    if (useFilters) {
        auto builder = std::make_unique<FilteredTreeBuilder>(path);
        if (maxDepth > 0) {
            builder->setMaxDepth(maxDepth);
        }
        return builder;
    }
    
    // ПРИОРИТЕТ 2: Если есть ограничение глубины - создаем DepthViewTreeBuilder
    if (maxDepth > 0) {
        return std::make_unique<DepthViewTreeBuilder>(path, maxDepth);
    }
    
    // ПРИОРИТЕТ 3: Многопоточность
    if ((threadCount > 1 || threadCount == 0)) {
        return std::make_unique<MultiThreadedTreeBuilder>(path, threadCount);
    }
    
    return std::make_unique<TreeBuilder>(path);
}

std::unique_ptr<TreeBuilder> BuilderFactory::create(const CommandLineOptions& options) {
    // ИСПРАВЛЕНИЕ: для GitHub используем githubUrl вместо path
    std::string targetPath = options.isGitHub ? options.githubUrl : options.path;
    
    return createBuilder(targetPath, 
                        options.useJSON,
                        options.maxDepth,
                        options.useFilteredBuilder,
                        options.threadCount);
}

void BuilderFactory::applySettings(const CommandLineOptions& options, TreeBuilder& builder) {
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