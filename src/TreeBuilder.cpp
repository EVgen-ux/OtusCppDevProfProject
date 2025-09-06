#include "TreeBuilder.h"

#define UNUSED(x) (void)(x)

TreeBuilder::TreeBuilder(const std::string& rootPath) 
    : BaseTreeBuilder(rootPath) {}

std::string TreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                      const std::string& connector) const {
    UNUSED(connector);
    
    if (info.isDirectory) {
        return info.name + " [DIR] | " + info.lastModified + " | " + info.permissions;
    } else {
        return info.name + " (" + info.sizeFormatted + ") | " + 
               info.lastModified + " | " + info.permissions;
    }
}

void TreeBuilder::traverseDirectory(const fs::path& path, 
                                  const std::string& prefix, 
                                  bool isLast,
                                  bool showHidden) {
    BaseTreeBuilder::traverseDirectory(path, prefix, isLast, showHidden);
}