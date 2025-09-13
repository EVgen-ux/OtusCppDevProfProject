#include "GitHubTreeBuilder.h"
#include "ColorManager.h"
#include "constants.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

GitHubTreeBuilder::GitHubTreeBuilder(const std::string& repoUrl, size_t maxDepth) 
    : repoUrl_(repoUrl), maxDepth_(maxDepth) {
    curl_ = curl_easy_init();
    isValid_ = parseRepoUrl() && (curl_ != nullptr);
    apiRequestCount_ = 0;
    
    if (curl_) {
        curl_easy_setopt(curl_, CURLOPT_USERAGENT, "Tree-Utility/1.0");
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    }
}

GitHubTreeBuilder::~GitHubTreeBuilder() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

void GitHubTreeBuilder::buildTree(bool showHidden) {
    (void)showHidden;
    
    treeLines_.clear();
    stats_ = Statistics{0, 0, 0};
    displayStats_ = DisplayStatistics{};
    
    if (!isValid_) {
        treeLines_.push_back("Ошибка: неверный URL GitHub репозитория");
        return;
    }
    
    treeLines_.push_back(ColorManager::getDirNameColor() + "[GITHUB] " + 
                        user_ + "/" + repo_ + " (" + branch_ + ")" + 
                        ColorManager::getReset());
    
    try {
        auto rootEntries = getGitHubTree(basePath_);
        if (rootEntries.empty()) {
            treeLines_.push_back("  └── (репозиторий пуст или недоступен)");
        } else {
            traverseGitHubTree(rootEntries, "", true, 1);
        }
    } catch (const std::exception& e) {
        treeLines_.push_back("  └── Ошибка: " + std::string(e.what()));
    }
}


void GitHubTreeBuilder::printTree() const {
    for (const auto& line : treeLines_) {
        std::cout << line << std::endl;
    }
}

ITreeBuilder::Statistics GitHubTreeBuilder::getStatistics() const {
    return stats_;
}

ITreeBuilder::DisplayStatistics GitHubTreeBuilder::getDisplayStatistics() const {
    return displayStats_;
}

const std::vector<std::string>& GitHubTreeBuilder::getTreeLines() const {
    return treeLines_;
}

size_t GitHubTreeBuilder::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool GitHubTreeBuilder::parseRepoUrl() {
    std::regex githubRegex(R"(https?://(?:www\.)?github\.com/([^/]+)/([^/]+)(?:/tree/([^/]+)(?:/(.*))?)?)");
    std::smatch matches;
    
    if (std::regex_match(repoUrl_, matches, githubRegex)) {
        user_ = matches[1];
        repo_ = matches[2];
        branch_ = matches[3].str().empty() ? "main" : matches[3].str();
        basePath_ = matches[4];
        
        apiUrl_ = "https://api.github.com/repos/" + user_ + "/" + repo_ + "/contents";
        if (!basePath_.empty()) {
            apiUrl_ += "/" + basePath_;
        }
        apiUrl_ += "?ref=" + branch_;
        
        return true;
    }
    return false;
}

std::string GitHubTreeBuilder::getApiUrl(const std::string& path) const {
    std::string url = "https://api.github.com/repos/" + user_ + "/" + repo_ + "/contents";
    if (!path.empty()) {
        url += "/" + path;
    }
    return url + "?ref=" + branch_;
}

std::vector<GitHubTreeBuilder::GitHubFileInfo> GitHubTreeBuilder::getGitHubTree(const std::string& path) {

    apiRequestCount_++;
    displayStats_.apiRequests = apiRequestCount_;

    std::string cacheKey = path.empty() ? "root" : path;
    if (treeCache_.find(cacheKey) != treeCache_.end()) {
        return treeCache_[cacheKey];
    }
    
    std::vector<GitHubFileInfo> result;
    std::string response;
    
    std::string url = getApiUrl(path);
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl_);
    
    if (res != CURLE_OK) {
        return result;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code != 200) {
        return result;
    }
    
    try {
        json data = json::parse(response);
        
        if (data.is_array()) {
            for (const auto& item : data) {
                GitHubFileInfo info;
                info.name = item.value("name", "");
                info.path = item.value("path", "");
                info.type = item.value("type", "");
                info.size = item.value("size", 0);
                info.sha = item.value("sha", "");
                info.url = item.value("url", "");
                
                // Получаем время последнего изменения из поля updated_at
                std::string updatedAt = item.value("updated_at", "");
                if (!updatedAt.empty()) {
                    info.lastModified = formatGitHubTime(updatedAt);
                } else {
                    // Если updated_at пустое, пытаемся получить время из коммитов
                    info.lastModified = getLastCommitTime(info.path);
                }
                
                if (info.type == "dir") {
                    info.size = 0;
                }
                
                result.push_back(info);
            }
        }
        
        treeCache_[cacheKey] = result;
    } catch (const std::exception&) {
        // Игнорируем ошибки парсинга
    }
    
    return result;
}

std::string GitHubTreeBuilder::getLastCommitTime(const std::string& path) {

    apiRequestCount_++;
    displayStats_.apiRequests = apiRequestCount_;
    
    // API для получения информации о коммитах
    std::string commitUrl = "https://api.github.com/repos/" + user_ + "/" + repo_ + 
                           "/commits?path=" + path + "&sha=" + branch_ + "&per_page=1";
    
    std::string response;
    
    curl_easy_setopt(curl_, CURLOPT_URL, commitUrl.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl_);
    
    if (res != CURLE_OK) {
        return "N/A";
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code != 200) {
        return "N/A";
    }
    
    try {
        json data = json::parse(response);
        
        if (data.is_array() && !data.empty()) {
            auto commit = data[0];
            auto commitInfo = commit.value("commit", json::object());
            auto author = commitInfo.value("author", json::object());
            std::string date = author.value("date", "");
            
            if (!date.empty()) {
                return formatGitHubTime(date);
            }
        }
    } catch (const std::exception&) {
        // Игнорируем ошибки парсинга
    }
    
    return "N/A";
}

std::string GitHubTreeBuilder::formatGitHubTime(const std::string& githubTime) const {
    if (githubTime.empty()) return "N/A";
    
    try {
        std::tm tm = {};
        std::istringstream ss(githubTime);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        
        if (ss.fail()) {
            // Пробуем другой формат (с Z на конце)
            ss.clear();
            ss.str(githubTime);
            char discard;
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S") >> discard;
        }
        
        if (!ss.fail()) {
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }
    } catch (...) {
        // В случае ошибки возвращаем исходную строку
    }
    
    // Если не удалось распарсить, возвращаем в укороченном формате
    if (githubTime.length() > 10) {
        return githubTime.substr(0, 10); // Только дата
    }
    return githubTime;
}



void GitHubTreeBuilder::traverseGitHubTree(const std::vector<GitHubFileInfo>& entries, 
                                         const std::string& prefix, 
                                         bool isLast,
                                         size_t currentDepth) {
    if (currentDepth > maxDepth_ + 2) {
        std::string connector = isLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
        treeLines_.push_back(prefix + connector + "(глубина ограничена)");
        return;
    }
    
    std::string newPrefix = prefix;
    if (isLast) {
        newPrefix += constants::TREE_SPACE;
    } else {
        newPrefix += constants::TREE_VERTICAL;
    }
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        bool entryIsLast = (i == entries.size() - 1);
        
        std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH 
                                          : constants::TREE_BRANCH;
        
        treeLines_.push_back(prefix + connector + formatTreeLine(entry, connector));
        
        if (entry.type == "dir") {
            stats_.totalDirectories++;
            displayStats_.displayedDirectories++;
            
            try {
                auto childEntries = getGitHubTree(entry.path);
                if (!childEntries.empty()) {
                    traverseGitHubTree(childEntries, newPrefix, entryIsLast, currentDepth + 1);
                }
            } catch (const std::exception&) {
                // Игнорируем ошибки доступа
            }
        } else if (entry.type == "file") {
            stats_.totalFiles++;
            stats_.totalSize += entry.size;
            displayStats_.displayedFiles++;
            displayStats_.displayedSize += entry.size;
        }
    }
}

std::string GitHubTreeBuilder::formatTreeLine(const GitHubFileInfo& info, const std::string& connector) const {
    (void)connector;
    
    if (info.type == "dir") {
        return ColorManager::getDirNameColor() + info.name + ColorManager::getReset() + " " + 
               ColorManager::getDirLabelColor() + "[DIR]" + ColorManager::getReset() + " | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset();
    } else {
        std::string nameColor = constants::WHITE;
        
        size_t dotPos = info.name.find_last_of(".");
        if (dotPos != std::string::npos) {
            std::string ext = info.name.substr(dotPos + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "c" || 
                ext == "java" || ext == "py" || ext == "js" || ext == "ts") {
                nameColor = constants::CODE_COLOR;
            } else if (ext == "md" || ext == "txt" || ext == "pdf" || ext == "doc" || ext == "docx") {
                nameColor = constants::DOCUMENT_COLOR;
            } else if (ext == "json" || ext == "yml" || ext == "yaml" || 
                      ext == "xml" || ext == "config" || ext == "ini") {
                nameColor = constants::CONFIG_COLOR;
            } else if (ext == "jpg" || ext == "png" || ext == "gif" || 
                      ext == "svg" || ext == "webp" || ext == "bmp") {
                nameColor = constants::IMAGE_COLOR;
            } else if (ext == "zip" || ext == "rar" || ext == "tar" || ext == "gz") {
                nameColor = constants::ARCHIVE_COLOR;
            }
        }
        
        std::string sizeStr;
        if (info.size > 0) {
            if (info.size < 1024) {
                sizeStr = " (" + std::to_string(info.size) + " B)";
            } else if (info.size < 1024 * 1024) {
                sizeStr = " (" + std::to_string(info.size / 1024) + " KB)";
            } else {
                sizeStr = " (" + std::to_string(info.size / (1024 * 1024)) + " MB)";
            }
        } else {
            sizeStr = "";
        }
        
        return nameColor + info.name + ColorManager::getReset() + sizeStr + " | " + 
               ColorManager::getDateColor() + info.lastModified + ColorManager::getReset();
    }
}

uint64_t GitHubTreeBuilder::parseSizeString(const std::string& sizeStr) const {
    try {
        return std::stoull(sizeStr);
    } catch (...) {
        return 0;
    }
}

std::string GitHubTreeBuilder::getFileDetailsFromApi(const std::string& fileUrl) {
    std::string response;
    
    curl_easy_setopt(curl_, CURLOPT_URL, fileUrl.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl_);
    
    return (res == CURLE_OK) ? response : "";
}