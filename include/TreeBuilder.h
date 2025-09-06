#pragma once

#include "BaseTreeBuilder.h"
#include <memory>

class TreeBuilder : public BaseTreeBuilder {
public:
    explicit TreeBuilder(const std::string& rootPath);
    
    // Переопределение методов для кастомизации
    std::string formatTreeLine(const FileSystem::FileInfo& info, 
                             const std::string& connector) const override;
    
protected:
    void traverseDirectory(const fs::path& path, 
                         const std::string& prefix, 
                         bool isLast,
                         bool showHidden) override;
};