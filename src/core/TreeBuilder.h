#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "FileSystem.h"
#include "ColorManager.h"

class TreeBuilder {
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
        int apiRequests = 0; 

        DisplayStatistics() : Statistics(), displayedFiles(0), displayedDirectories(0), 
                         displayedSize(0), hiddenByDepth(0), hiddenObjects(0), apiRequests(0) {}
    };
    
    explicit TreeBuilder(const std::string& rootPath);
    virtual ~TreeBuilder() = default;
    
    virtual void buildTree(bool showHidden = false);
    virtual void printTree() const;
    virtual Statistics getStatistics() const;
    virtual DisplayStatistics getDisplayStatistics() const;
    virtual const std::vector<std::string>& getTreeLines() const;
    
protected:
    std::filesystem::path rootPath_;
    Statistics stats_;
    DisplayStatistics displayStats_;
    std::vector<std::string> treeLines_;
    size_t hiddenObjectsCount_ = 0;
    
    virtual void traverseDirectory(const std::filesystem::path& path, 
                                 const std::string& prefix, 
                                 bool isLast,
                                 bool showHidden,
                                 bool isRoot = false);
    
    virtual std::string formatTreeLine(const FileSystem::FileInfo& info, 
                                     const std::string& connector) const;
};