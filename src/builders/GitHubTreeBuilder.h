#pragma once
#include "TreeBuilder.h"
#include <string>
#include <vector>
#include <curl/curl.h>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class GitHubTreeBuilder : public TreeBuilder {
public:
    GitHubTreeBuilder(const std::string& repoUrl, size_t maxDepth = 3);
    ~GitHubTreeBuilder();
    
    void buildTree(bool showHidden = false) override;
    void printTree() const override;
    
    bool isValid() const { return isValid_; }
    void setMaxDepth(size_t maxDepth) { maxDepth_ = maxDepth; }

    using TreeBuilder::formatTreeLine;

private:
    struct GitHubFileInfo {
        std::string path;
        std::string name;
        std::string type;
        uint64_t size = 0;
        std::string sha;
        std::string url;
        std::string lastModified;
    };
    
    std::string repoUrl_;
    std::string apiUrl_;
    std::string user_;
    std::string repo_;
    std::string branch_;
    std::string basePath_;
    bool isValid_ = false;
    CURL* curl_ = nullptr;
    size_t maxDepth_;
    int apiRequestCount_ = 0;
    std::map<std::string, std::vector<GitHubFileInfo>> treeCache_;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
    
    bool parseRepoUrl();
    std::string getApiUrl(const std::string& path = "") const;
    std::vector<GitHubFileInfo> getGitHubTree(const std::string& path = "");
    std::string formatTreeLine(const GitHubFileInfo& info, const std::string& connector) const;
    void traverseGitHubTree(const std::vector<GitHubFileInfo>& entries, 
                          const std::string& prefix, 
                          bool isLast,
                          size_t currentDepth = 0);
    
    std::string getLastCommitTime(const std::string& path);
    std::string formatGitHubTime(const std::string& githubTime) const;
};