#pragma once
#include "TreeBuilder.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JSONTreeBuilder : public TreeBuilder {
public:
    explicit JSONTreeBuilder(const std::string& rootPath);
    
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    std::string getJSON() const;
    
private:
    json jsonData_;
    
    json traverseDirectoryJSON(const std::filesystem::path& path, 
                             bool showHidden,
                             bool isRoot = false);
    json fileInfoToJSON(const FileSystem::FileInfo& info);
};