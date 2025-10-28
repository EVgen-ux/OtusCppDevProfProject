#include "MultiThreadedTreeBuilder.h"
#include <iostream>
#include <algorithm>
#include <future>
#include <thread>

namespace fs = std::filesystem;

MultiThreadedTreeBuilder::MultiThreadedTreeBuilder(const std::string& rootPath, size_t threadCount)
    : TreeBuilder(rootPath), threadCount_(threadCount) {
    
    if (threadCount_ == 0) {
        unsigned int hwThreads = std::thread::hardware_concurrency();
        threadCount_ = (hwThreads == 0) ? 2 : static_cast<size_t>(hwThreads);
    }
    
    std::cout << "Используется потоков: " << threadCount_ << std::endl;
}
MultiThreadedTreeBuilder::~MultiThreadedTreeBuilder() {
    stopProcessing_ = true;
    stopThreadPool();
}

void MultiThreadedTreeBuilder::startThreadPool() {
    for (size_t i = 0; i < threadCount_; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex_);
                    this->condition_.wait(lock, [this] {
                        return this->stopPool_ || !this->tasks_.empty();
                    });
                    if (this->stopPool_ && this->tasks_.empty()) {
                        return;
                    }
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
            }
        });
    }
}

void MultiThreadedTreeBuilder::stopThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stopPool_ = true;
    }
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

void MultiThreadedTreeBuilder::addTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if (stopPool_) return;
        tasks_.emplace(std::move(task));
    }
    condition_.notify_one();
}

void MultiThreadedTreeBuilder::buildTree(bool showHidden) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    hiddenObjectsCount_ = 0;
    stopProcessing_ = false;
    stopPool_ = false;
    
    startThreadPool();
    
    treeLines_.push_back(ColorManager::getDirNameColor() + "[DIR]" + ColorManager::getReset());
    traverseDirectoryHybrid(rootPath_, "", true, showHidden, true);
    
    // Ждем завершения всех задач
    stopThreadPool();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    displayStats_.buildTimeMicroseconds = 
        std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
}

void MultiThreadedTreeBuilder::traverseDirectoryHybrid(const fs::path& path, 
                                                     const std::string& prefix, 
                                                     bool isLast,
                                                     bool showHidden,
                                                     bool isRoot) {
    if (stopProcessing_) return;
    
    if (!isRoot) {
        auto info = FileSystem::getFileInfo(path);
        std::string connector = isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        
        {
            std::lock_guard<std::mutex> lock(treeLinesMutex_);
            treeLines_.push_back(prefix + connector + TreeBuilder::formatTreeLine(info, connector));
        }
        
        stats_.totalDirectories++;
        displayStats_.displayedDirectories++;
    }
    
    std::string newPrefix = prefix;
    if (isLast) {
        newPrefix += constants::TREE_SPACE;
    } else {
        newPrefix += constants::TREE_VERTICAL;
    }
    
    std::vector<fs::directory_entry> entries;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (stopProcessing_) return;
            
            bool isHidden = FileSystem::isHidden(entry.path());
            if (!isHidden || showHidden) {
                entries.push_back(entry);
            } else {
                hiddenObjectsCount_++;
            }
        }
    } catch (const fs::filesystem_error&) {
        return;
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        bool a_dir = a.is_directory();
        bool b_dir = b.is_directory();
        if (a_dir != b_dir) {
            return a_dir > b_dir; // Директории first
        }
        return a.path().filename().string() < b.path().filename().string();
    });
    
    // РАЗДЕЛЯЕМ НА ДИРЕКТОРИИ И ФАЙЛЫ
    std::vector<fs::directory_entry> directories;
    std::vector<fs::directory_entry> files;
    
    for (const auto& entry : entries) {
        if (entry.is_directory()) {
            directories.push_back(entry);
        } else {
            files.push_back(entry);
        }
    }
    
    // для маленьких директорий используем однопоточность
    if (files.size() < 10 || threadCount_ == 1) {
        for (size_t i = 0; i < entries.size(); ++i) {
            if (stopProcessing_) break;
            
            const auto& entry = entries[i];
            bool entryIsLast = (i == entries.size() - 1);
            
            if (entry.is_directory()) {
                traverseDirectoryHybrid(entry.path(), newPrefix, entryIsLast, showHidden, false);
            } else {
                auto info = FileSystem::getFileInfo(entry.path());
                std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
                
                {
                    std::lock_guard<std::mutex> lock(treeLinesMutex_);
                    treeLines_.push_back(newPrefix + connector + TreeBuilder::formatTreeLine(info, connector));
                }
                
                stats_.totalFiles++;
                stats_.totalSize += info.size;
                displayStats_.displayedFiles++;
                displayStats_.displayedSize += info.size;
            }
        }
    } else {
        
        std::vector<std::future<FileSystem::FileInfo>> fileFutures;
        fileFutures.reserve(files.size());
        
        for (const auto& file : files) {
            if (stopProcessing_) break;
            
            auto promise = std::make_shared<std::promise<FileSystem::FileInfo>>();
            fileFutures.push_back(promise->get_future());
            
            addTask([file, promise, this]() {
                if (stopProcessing_) {
                    promise->set_value(FileSystem::FileInfo());
                    return;
                }
                promise->set_value(FileSystem::getFileInfo(file.path()));
            });
        }
        
        // Обрабатываем директории пока файлы считаются
        for (size_t i = 0; i < directories.size(); ++i) {
            if (stopProcessing_) break;
            
            bool entryIsLast = (i == directories.size() - 1 && files.empty());
            traverseDirectoryHybrid(directories[i].path(), newPrefix, entryIsLast, showHidden, false);
        }
        
        // Собираем результаты файлов
        for (size_t i = 0; i < fileFutures.size(); ++i) {
            if (stopProcessing_) break;
            
            auto info = fileFutures[i].get();
            if (info.name.empty()) continue;
            
            bool entryIsLast = (directories.size() + i == entries.size() - 1);
            std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
            
            {
                std::lock_guard<std::mutex> lock(treeLinesMutex_);
                treeLines_.push_back(newPrefix + connector + TreeBuilder::formatTreeLine(info, connector));
            }
            
            stats_.totalFiles++;
            stats_.totalSize += info.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += info.size;
        }
    }
}