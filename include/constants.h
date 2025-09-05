#pragma once

#include <string>

namespace constants {
    const std::string VERSION = "1.0.0";
    const std::string AUTHOR = "EVgen-ux";
    
    // Символы для отрисовки дерева
    const std::string TREE_BRANCH = "├── ";
    const std::string TREE_LAST_BRANCH = "└── ";
    const std::string TREE_VERTICAL = "│   ";
    const std::string TREE_SPACE = "    ";
    
    // Размеры файлов
    const uint64_t KB = 1024;
    const uint64_t MB = 1024 * KB;
    const uint64_t GB = 1024 * MB;
    const uint64_t TB = 1024 * GB;
}

