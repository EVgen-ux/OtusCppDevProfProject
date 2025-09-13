#include "ITreeBuilder.h"
#include "TreeBuilder.h"
#include "GitHubTreeBuilder.h"

std::unique_ptr<ITreeBuilder> ITreeBuilder::create(const std::string& rootPath) {
    if (rootPath.find("github.com") != std::string::npos) {
        return std::unique_ptr<ITreeBuilder>(new GitHubTreeBuilder(rootPath));
    } else {
        return std::unique_ptr<ITreeBuilder>(new TreeBuilder(rootPath));
    }
}