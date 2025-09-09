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
    
    struct DisplayStatistics : Statistics {
        size_t displayedFiles = 0;
        size_t displayedDirectories = 0;
        uint64_t displayedSize = 0;
        size_t hiddenByDepth = 0;
        size_t hiddenObjects = 0;

        DisplayStatistics() : Statistics(), displayedFiles(0), displayedDirectories(0), 
                         displayedSize(0), hiddenByDepth(0) {}
    };
    
    virtual ~ITreeBuilder() = default;
    
    virtual void buildTree(bool showHidden = false) = 0;
    virtual void printTree() const = 0;
    virtual Statistics getStatistics() const = 0;
    virtual DisplayStatistics getDisplayStatistics() const = 0; // Новая функция
    virtual const std::vector<std::string>& getTreeLines() const = 0;
    
    // Фабричный метод
    static std::unique_ptr<ITreeBuilder> create(const std::string& rootPath);
};