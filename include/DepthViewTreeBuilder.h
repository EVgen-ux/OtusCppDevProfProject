#pragma once

#include "TreeBuilder.h"

class DepthViewTreeBuilder : public TreeBuilder {
public:
    DepthViewTreeBuilder(const std::string& rootPath, size_t maxDepth = 0);
    
    void buildTree(bool showHidden = false) override;
    
    void setMaxDepth(size_t maxDepth);
    size_t getMaxDepth() const;

private:
    size_t maxDepth_;
    size_t currentDepth_;
    
    void traverseDirectory(const fs::path& path, 
                          const std::string& prefix, 
                          bool isLast,
                          bool showHidden) override;
};