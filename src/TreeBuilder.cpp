#include "TreeBuilder.h"
#include <algorithm>

TreeBuilder::TreeBuilder(const std::string& rootPath) : BaseTreeBuilder(rootPath) {}

std::string TreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                      const std::string& /*connector*/) const {
    std::string line = info.name;
    if (info.isDirectory) {
        line += " [DIR]";
    } else {
        line += " (" + info.sizeFormatted + ")";
    }
    line += " | " + info.lastModified + " | " + info.permissions;
    return line;
}

void TreeBuilder::traverseDirectory(const fs::path& path, 
                                  const std::string& prefix, 
                                  bool isLast,
                                  bool showHidden) {
    auto filter = [](const fs::directory_entry& entry, bool showHidden) {
        return showHidden || !FileSystem::isHidden(entry.path());
    };
    
    auto sort = [](const fs::directory_entry& a, const fs::directory_entry& b) {
        bool aIsDir = a.is_directory();
        bool bIsDir = b.is_directory();
        if (aIsDir != bIsDir) return aIsDir > bIsDir;
        return a.path().filename() < b.path().filename();
    };
    
    processDirectoryEntries(path, filter, sort, showHidden, prefix, isLast);
}