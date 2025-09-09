#include "JSONTreeBuilder.h"
#include <iostream>
#include <regex>

JSONTreeBuilder::JSONTreeBuilder(std::unique_ptr<ITreeBuilder> builder)
    : builder_(std::move(builder)) {}

void JSONTreeBuilder::buildTree(bool showHidden) {
    builder_->buildTree(showHidden);
    convertToJSON(builder_->getTreeLines(), jsonTree_);
}

void JSONTreeBuilder::printTree() const {
    std::cout << jsonTree_.dump(2) << std::endl;
}

ITreeBuilder::Statistics JSONTreeBuilder::getStatistics() const {
    return builder_->getStatistics();
}

ITreeBuilder::DisplayStatistics JSONTreeBuilder::getDisplayStatistics() const {
    return builder_->getDisplayStatistics();
}

const std::vector<std::string>& JSONTreeBuilder::getTreeLines() const {
    return builder_->getTreeLines();
}

std::string JSONTreeBuilder::getJSON() const {
    return jsonTree_.dump(2);
}

void JSONTreeBuilder::convertToJSON(const std::vector<std::string>& treeLines, json& output) {
    if (treeLines.empty()) {
        output = json::object();
        return;
    }
    
    json root;
    root["name"] = ".";
    root["type"] = "directory";
    root["children"] = json::array();
    
    // Стек для отслеживания текущей позиции в дереве
    std::vector<json*> stack;
    stack.push_back(&root);
    
    // Регулярные выражения для парсинга строк
    std::regex dirRegex(R"((.*)\[DIR\].*\|\s*(.*)\s*\|\s*(.*))");
    std::regex fileRegex(R"((.*)\(([^)]+)\).*\|\s*(.*)\s*\|\s*(.*))");
    std::regex hiddenRegex(R"((.*)\[DIR\]\s*\(содержимое скрыто\).*\|\s*(.*)\s*\|\s*(.*))");
    
    for (size_t i = 1; i < treeLines.size(); ++i) { // Пропускаем корневую директорию
        const auto& line = treeLines[i];
        
        // Определяем уровень вложенности по отступам - используем ASCII символы
        size_t level = 0;
        size_t pos = 0;
        while (pos < line.size() && (line[pos] == ' ' || line[pos] == '|' || 
               line[pos] == '+' || line[pos] == '\\')) {
            if (line[pos] == ' ' || line[pos] == '|') level++;
            pos += 4; // Каждый уровень отступа - 4 символа
        }
        
        // Убираем лишние уровни из стека
        while (stack.size() > level + 1) {
            stack.pop_back();
        }
        
        std::smatch matches;
        json item;
        
        if (std::regex_match(line, matches, hiddenRegex)) {
            // Директория со скрытым содержимым
            item["name"] = matches[1].str();
            item["type"] = "directory";
            item["last_modified"] = matches[2].str();
            item["permissions"] = matches[3].str();
            item["content_hidden"] = true;
        }
        else if (std::regex_match(line, matches, dirRegex)) {
            // Обычная директория
            item["name"] = matches[1].str();
            item["type"] = "directory";
            item["last_modified"] = matches[2].str();
            item["permissions"] = matches[3].str();
            item["children"] = json::array();
        }
        else if (std::regex_match(line, matches, fileRegex)) {
            // Файл
            item["name"] = matches[1].str();
            item["type"] = "file";
            item["size"] = matches[2].str();
            item["last_modified"] = matches[3].str();
            item["permissions"] = matches[4].str();
        }
        else {
            // Неизвестный формат
            item["name"] = line.substr(pos);
            item["type"] = "unknown";
        }
        
        // Добавляем элемент в текущий уровень
        (*stack.back())["children"].push_back(item);
        
        // Если это директория, добавляем в стек для дочерних элементов
        if (item.contains("children")) {
            stack.push_back(&(*stack.back())["children"].back());
        }
    }
    
    output = root;
}