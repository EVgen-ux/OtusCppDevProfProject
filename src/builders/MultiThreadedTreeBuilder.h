#pragma once
#include "TreeBuilder.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <thread>

class MultiThreadedTreeBuilder : public TreeBuilder {
public:
    explicit MultiThreadedTreeBuilder(const std::string& rootPath, size_t threadCount = 0);
    ~MultiThreadedTreeBuilder();
    
    void buildTree(bool showHidden = false) override;

private:
    size_t threadCount_;
    std::atomic<bool> stopProcessing_{false};
    mutable std::mutex treeLinesMutex_;
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stopPool_{false};
    
    void startThreadPool();
    void stopThreadPool();
    void addTask(std::function<void()> task);
    
    void traverseDirectoryHybrid(const std::filesystem::path& path, 
                                const std::string& prefix, 
                                bool isLast,
                                bool showHidden,
                                bool isRoot = false);
};