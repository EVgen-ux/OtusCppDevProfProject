#pragma once
#include "TreeBuilder.h"
#include <atomic>

class MultiThreadedTreeBuilder : public TreeBuilder {
public:
    explicit MultiThreadedTreeBuilder(const std::string& rootPath, size_t threadCount = 0);
    ~MultiThreadedTreeBuilder();
    
    void buildTree(bool showHidden = false) override;

private:
    size_t threadCount_;
    std::atomic<bool> stopProcessing_{false};
    
    void traverseDirectoryHybrid(const std::filesystem::path& path, 
                                const std::string& prefix, 
                                bool isLast,
                                bool showHidden,
                                bool isRoot = false);
    
    std::string formatTreeLine(const FileSystem::FileInfo& info, 
                              const std::string& connector) const override;
};