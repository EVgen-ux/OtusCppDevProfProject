#pragma once
#include "TreeBuilder.h"
#include <sstream>
#include <iomanip>


class JSONTreeBuilder : public TreeBuilder {
public:
    explicit JSONTreeBuilder(const std::string& rootPath);
    
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    
    std::string getJSON() const;

private:
    std::string jsonOutput_;
    
    void buildJSONTree();
    void appendDirectoryToJSON(const std::filesystem::path& path, bool showHidden, 
                              std::stringstream& json, int depth, int maxDepth);
    std::string escapeJSONString(const std::string& str) const;
};