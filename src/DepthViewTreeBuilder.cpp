#include "DepthViewTreeBuilder.h"
#include <iostream>

DepthViewTreeBuilder::DepthViewTreeBuilder(const std::string& rootPath, size_t maxDepth)
    : TreeBuilder(rootPath), maxDepth_(maxDepth), currentDepth_(0) {}

void DepthViewTreeBuilder::setMaxDepth(size_t maxDepth) {
    maxDepth_ = maxDepth;
}

size_t DepthViewTreeBuilder::getMaxDepth() const {
    return maxDepth_;
}

void DepthViewTreeBuilder::buildTree(bool showHidden) {
    currentDepth_ = 0;
    TreeBuilder::buildTree(showHidden);
}

void DepthViewTreeBuilder::traverseDirectory(const fs::path& path, 
                                             const std::string& prefix, 
                                             bool isLast,
                                             bool showHidden) {
    currentDepth_++;
    
    if (maxDepth_ > 0 && currentDepth_ > maxDepth_) {
        getTreeLines().push_back(prefix + (isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH) + "[...]");
        currentDepth_--;
        return;
    }
    
    try {
        TreeBuilder::traverseDirectory(path, prefix, isLast, showHidden);
    }
    catch (...) {
        currentDepth_--;
        throw;
    }
    
    currentDepth_--;
}