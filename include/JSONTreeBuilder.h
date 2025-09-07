#pragma once

#include "ITreeBuilder.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

class JSONTreeBuilder : public ITreeBuilder {
public:
    JSONTreeBuilder(std::unique_ptr<ITreeBuilder> builder);
    
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    Statistics getStatistics() const override;
    DisplayStatistics getDisplayStatistics() const override;
    const std::vector<std::string>& getTreeLines() const override;
    
    std::string getJSON() const;
    
private:
    std::unique_ptr<ITreeBuilder> builder_;
    json jsonTree_;
    
    void convertToJSON(const std::vector<std::string>& treeLines, json& output);
};