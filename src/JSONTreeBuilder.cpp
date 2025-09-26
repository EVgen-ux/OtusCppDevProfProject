#include "JSONTreeBuilder.h"
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

JSONTreeBuilder::JSONTreeBuilder(const std::string& rootPath) 
    : TreeBuilder(rootPath) {}

void JSONTreeBuilder::buildTree(bool showHidden) {
    // Сначала строим обычное дерево для статистики
    TreeBuilder::buildTree(showHidden);  
    // Затем генерируем JSON
    buildJSONTree();
}

void JSONTreeBuilder::buildJSONTree() {
    std::stringstream json;
    
    json << "{\n";
    json << "  \"path\": \"" << escapeJSONString(rootPath_.string()) << "\",\n";
    json << "  \"name\": \"" << escapeJSONString(rootPath_.filename().string()) << "\",\n";
    json << "  \"type\": \"directory\",\n";
    
    auto stats = getStatistics();
    json << "  \"statistics\": {\n";
    json << "    \"directories\": " << stats.totalDirectories << ",\n";
    json << "    \"files\": " << stats.totalFiles << ",\n";
    json << "    \"totalSize\": " << stats.totalSize << ",\n";
    json << "    \"totalSizeFormatted\": \"" << FileSystem::formatSize(stats.totalSize) << "\"\n";
    json << "  },\n";
    
    json << "  \"contents\": [\n";
    
    try {
        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(rootPath_)) {
            entries.push_back(entry);
        }
        
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory()) {
                return a.is_directory() > b.is_directory();
            }
            return a.path().filename().string() < b.path().filename().string();
        });
        
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            auto info = FileSystem::getFileInfo(entry.path());
            
            json << "    {\n";
            json << "      \"name\": \"" << escapeJSONString(info.name) << "\",\n";
            json << "      \"type\": \"" << (info.isDirectory ? "directory" : "file") << "\",\n";
            
            if (info.isDirectory) {
                json << "      \"size\": 0,\n";
                json << "      \"sizeFormatted\": \"0 B\"\n";
            } else {
                json << "      \"size\": " << info.size << ",\n";
                json << "      \"sizeFormatted\": \"" << escapeJSONString(info.sizeFormatted) << "\"\n";
            }
            
            json << "    }";
            
            if (i < entries.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
    } catch (const fs::filesystem_error&) {
        // Игнорируем ошибки доступа
    }
    
    json << "  ]\n";
    json << "}";
    
    jsonOutput_ = json.str();
}

void JSONTreeBuilder::printTree() const {
    std::cout << jsonOutput_ << std::endl;
}

std::string JSONTreeBuilder::getJSON() const {
    return jsonOutput_;
}

std::string JSONTreeBuilder::escapeJSONString(const std::string& str) const {
    std::stringstream escaped;
    
    for (char c : str) {
        switch (c) {
            case '"': escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) == 0x7f) {
                    escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') 
                           << static_cast<int>(c);
                } else {
                    escaped << c;
                }
                break;
        }
    }
    
    return escaped.str();
}