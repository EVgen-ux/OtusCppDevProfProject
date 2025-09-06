#include "ITreeBuilder.h"
#include "TreeBuilder.h"

std::unique_ptr<ITreeBuilder> ITreeBuilder::create(const std::string& rootPath) {
    return std::unique_ptr<ITreeBuilder>(new TreeBuilder(rootPath));
}