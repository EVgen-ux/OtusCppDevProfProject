#include "TreeBuilder.h"
#include "ColorManager.h" // Добавьте этот include

#define UNUSED(x) (void)(x)

TreeBuilder::TreeBuilder(const std::string& rootPath) 
    : BaseTreeBuilder(rootPath) {}

std::string TreeBuilder::formatTreeLine(const FileSystem::FileInfo& info, 
                                      const std::string& connector) const {
    UNUSED(connector);
    
    std::string nameColor = FileSystem::getFileColor(info);
    
    if (info.isDirectory) {
        return nameColor + info.name + ColorManager::getReset() + " " + 
               ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
               ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset();
    } else {
        return nameColor + info.name + ColorManager::getReset() + " (" + 
               ColorManager::getSizeColor() + info.sizeFormatted + ColorManager::getReset() + ") | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset() + " | " + 
               ColorManager::getPermissionsColor() + info.permissions + ColorManager::getReset();
    }
}

void TreeBuilder::traverseDirectory(const fs::path& path, 
                                  const std::string& prefix, 
                                  bool isLast,
                                  bool showHidden,
                                  bool isRoot) {
    BaseTreeBuilder::traverseDirectory(path, prefix, isLast, showHidden, isRoot);
}