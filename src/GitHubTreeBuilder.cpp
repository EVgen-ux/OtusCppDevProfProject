#include "GitHubTreeBuilder.h"
#include "ColorManager.h"
#include "Constants.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <algorithm>

GitHubTreeBuilder::GitHubTreeBuilder(const std::string& repoUrl, size_t maxDepth) 
    : TreeBuilder(""), repoUrl_(repoUrl), maxDepth_(maxDepth) {
    curl_ = curl_easy_init();
    isValid_ = parseRepoUrl() && (curl_ != nullptr);
    apiRequestCount_ = 0;
    
    if (curl_) {
        curl_easy_setopt(curl_, CURLOPT_USERAGENT, "Tree-Utility/2.0");
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
        return true;
    }
    return false;
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
                
                std::string updatedAt = item.value("updated_at", "");
                if (!updatedAt.empty()) {
                    info.lastModified = formatGitHubTime(updatedAt);
                } else {
                    info.lastModified = getLastCommitTime(info.path);
                }
                
                if (info.type == "dir") {
                    info.size = 0;
                }
                
                result.push_back(info);
            }
        }
        
        treeCache_[cacheKey] = result;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга JSON: " << e.what() << std::endl;
    }
    
    return result;
}

std::string GitHubTreeBuilder::getLastCommitTime(const std::string& path) {
    apiRequestCount_++;
    displayStats_.apiRequests = apiRequestCount_;
    
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
    }
    
    if (githubTime.length() > 10) {
        return githubTime.substr(0, 10);
    }
    return githubTime;
}

std::string GitHubTreeBuilder::getApiUrl(const std::string& path) const {
    std::string url = "https://api.github.com/repos/" + user_ + "/" + repo_ + "/contents";
    if (!path.empty()) {
        url += "/" + path;
    }
    return url + "?ref=" + branch_;
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
        
        std::string connector = entryIsLast ? constants::TREE_LAST_BRANCH : constants::TREE_BRANCH;
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
    
    const bool colorsEnabled = ColorManager::areColorsEnabled();
    std::stringstream line;
    
    if (colorsEnabled) {
        if (info.type == "dir") {
            line << ColorManager::getDirNameColor();
        } else {
            std::string ext = "";
            size_t dotPos = info.name.find_last_of(".");
            if (dotPos != std::string::npos && dotPos < info.name.length() - 1) {
                ext = info.name.substr(dotPos + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            }
            
            if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "c" || 
                ext == "java" || ext == "py" || ext == "js" || ext == "ts") {
                line << constants::CODE_COLOR;
            } else if (ext == "md" || ext == "txt" || ext == "pdf") {
                line << constants::DOCUMENT_COLOR;
            } else if (ext == "json" || ext == "yml" || ext == "yaml" || ext == "xml") {
                line << constants::CONFIG_COLOR;
            } else if (ext == "jpg" || ext == "png" || ext == "gif" || ext == "svg") {
                line << constants::IMAGE_COLOR;
            } else if (ext == "zip" || ext == "rar" || ext == "tar" || ext == "gz") {
                line << constants::ARCHIVE_COLOR;
            } else {
                line << constants::WHITE;
            }
        }
    }
    
    line << info.name;
    if (colorsEnabled) line << ColorManager::getReset();
    
    if (info.type == "dir") {
        line << " ";
        if (colorsEnabled) line << ColorManager::getDirLabelColor();
        line << "[DIR]";
        if (colorsEnabled) line << ColorManager::getReset();
    }
    else if (info.size > 0) {
        line << " (";
        if (colorsEnabled) line << ColorManager::getSizeColor();
        

        if (info.size < 1024) {
            line << info.size << " B";
        } else if (info.size < 1024 * 1024) {
            line << (info.size / 1024) << " KB";
        } else if (info.size < 1024 * 1024 * 1024) {
            line << (info.size / (1024 * 1024)) << " MB";
        } else {
            line << (info.size / (1024 * 1024 * 1024)) << " GB";
        }
        
        if (colorsEnabled) line << ColorManager::getReset();
        line << ")";
    }
    
    // Дата модификации
    line << " | ";
    if (colorsEnabled) line << ColorManager::getDateColor();
    line << info.lastModified;
    if (colorsEnabled) line << ColorManager::getReset();
    
    return line.str();
}
