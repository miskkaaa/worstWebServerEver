#include <string>
#include <vector>

namespace cfg {
    struct Config {
        std::string page404 = "/404.html";
        std::string page400 = "/400.html";
        std::vector<std::string> hiddenFiles = {".wsecfg"}; // i have to hardcode this
        std::vector<std::string> hiddenFolders;
    };

    Config loadConfig(const std::string& configPath);
    bool isHiddenFile(const std::string& path, const Config& cfg);
    bool isHiddenFolder(const std::string& path, const Config& cfg);
}