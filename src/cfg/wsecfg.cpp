#include "../includes/wsecfg.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace cfg {
    Config loadConfig(const std::string& configPath) {
        Config config;
        std::ifstream cfgFile(configPath);

        if (!cfgFile.is_open()) return config;

        std::string line;

        while (std::getline(cfgFile, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));

            if (line.empty() || line[0] == '#') continue;

            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            std::size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);

            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t") + 1);

            if (key == "404page") {
                config.page404 = val;

            } else if (key == "400page") {
                config.page400 = val;

            } else if (key == "hiddenfiles") {
                std::stringstream ss(val);
                std::string item;

                while (std::getline(ss, item, ',')) {
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);

                    if (!item.empty()) {
                        config.hiddenFiles.push_back(item);
                    }
                }

            } else if (key == "hiddenfolders") {
                std::stringstream ss(val);
                std::string item;

                while (std::getline(ss, item, ',')) {
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);

                    if (!item.empty()) {
                        config.hiddenFolders.push_back(item);
                    }
                }
            }
        }

        return config;
    }

    bool isHiddenFile(const std::string& path, const Config& cfg) {
        std::filesystem::path p(path);
        std::string filename = p.filename().string();

        for (const auto& hidden : cfg.hiddenFiles) {
            if (filename == hidden) return true;
        }
        
        return false;
    }

    bool isHiddenFolder(const std::string& path, const Config& cfg) {
        for (const auto& folder : cfg.hiddenFolders) {
            std::string formattedFolder = folder;

            if (formattedFolder.empty()) continue;

            if (formattedFolder.front() != '/') {
                formattedFolder = "/" + formattedFolder;
            }

            if (path == formattedFolder ||
                path.find(formattedFolder + "/") == 0) {
                return true;
            }
        }

        return false;
    }
}