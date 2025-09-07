#pragma once

#include <string>
#include <cstdint>

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
    
    // ANSI цветовые коды
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    
    // Основные цвета
    const std::string BLACK = "\033[30m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string BRIGHT_YELLOW = "\033[93m";
    
    // Специальные цвета для элементов
    const std::string DIR_NAME_COLOR = BLUE + BOLD;      // Синий для имен директорий
    const std::string DIR_LABEL_COLOR = CYAN;            // Голубой для [DIR]
    const std::string SIZE_COLOR = GREEN;                // Зеленый для размеров
    const std::string DATE_COLOR = YELLOW;               // Желтый для дат
    const std::string PERMISSIONS_COLOR = MAGENTA;       // Пурпурный для прав доступа
    const std::string HIDDEN_CONTENT_COLOR = BRIGHT_YELLOW; // Ярко-желтый для "содержимое скрыто"
    
    // Цвета для типов файлов
    const std::string EXECUTABLE_COLOR = GREEN + BOLD;
    const std::string SYMLINK_COLOR = CYAN;
    const std::string IMAGE_COLOR = MAGENTA;
    const std::string ARCHIVE_COLOR = RED;
    const std::string CONFIG_COLOR = YELLOW;
    const std::string DOCUMENT_COLOR = WHITE;
    const std::string CODE_COLOR = GREEN;
    const std::string VIDEO_COLOR = MAGENTA + BOLD;
    const std::string AUDIO_COLOR = CYAN + BOLD;
    const std::string HIDDEN_COLOR = BLACK + BOLD;
}