#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class ITreeBuilder {
public:
    struct Statistics {
        size_t totalFiles = 0;
        size_t totalDirectories = 0;
        uint64_t totalSize = 0;
    };
    
    virtual ~ITreeBuilder() = default;
    
    virtual void buildTree(bool showHidden = false) = 0;
    virtual void printTree() const = 0;
    virtual Statistics getStatistics() const = 0;
    virtual const std::vector<std::string>& getTreeLines() const = 0;
    
    // Фабричный метод
    static std::unique_ptr<ITreeBuilder> create(const std::string& rootPath);
};