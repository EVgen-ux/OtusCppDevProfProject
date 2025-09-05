#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class TreeBuilder {
public:
    struct Statistics {
        size_t totalFiles = 0;
        size_t totalDirectories = 0;
        uint64_t totalSize = 0;
    };
    
    TreeBuilder(const std::string& rootPath);
    
    void buildTree(bool showHidden = false);
    void printTree() const;
    Statistics getStatistics() const;
    
private:
    fs::path rootPath_;
    Statistics stats_;
    std::vector<std::string> treeLines_;
    
    void traverseDirectory(const fs::path& path, 
                          const std::string& prefix, 
                          bool isLast,
                          bool showHidden);
    bool shouldIncludeFile(const fs::path& path, bool showHidden) const;
};
