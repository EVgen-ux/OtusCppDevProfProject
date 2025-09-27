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
    std::string escapeJSONString(const std::string& str) const;
};