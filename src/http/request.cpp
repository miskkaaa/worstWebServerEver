#include "../includes/request.hpp"
#include <sstream>
namespace http {
    bool parse(const std::string& raw, Request& request) {
        const std::size_t end = raw.find("\r\n\r\n");
        if (end == std::string::npos) {
            return false;
        }

        const std::string sect = raw.substr(0, end);
        request.body = raw.substr(end + 4);
        std::istringstream stream(sect);

        std::string reqline;
        std::string line;

        if (!std::getline(stream, reqline)) return false;
        if (!reqline.empty() && reqline.back() == '\r') reqline.pop_back();

        std::istringstream reqlinestream(reqline);
        std::string extra;

        if (!(reqlinestream >> request.method >> request.path >> request.version)) return false; 
        if (reqlinestream >> extra) return false;
        if (request.method.empty() || request.path.empty()) return false;
        if (request.path[0] != '/') return false;

        if (request.version != "HTTP/1.0" &&
            request.version != "HTTP/1.1") {
            return false;
        }

        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) continue;

            const std::size_t colon = line.find(':');

            if (colon == std::string::npos) return false;
            if (colon == 0) return false;

            std::string nam = line.substr(0, colon);
            std::string val = line.substr(colon + 1);

            if (nam.back() == ' ' || nam.back() == '\t') return false;

            for (unsigned char c : nam) {
                if (c <= 32 || c >= 127) {
                    return false;
                }
            }

            while (!val.empty() &&
                   (val.front() == ' ' || val.front() == '\t')) {
                val.erase(val.begin());
            }

            for (unsigned char c : val) {
                if (c < 32 && c != '\t') {
                    return false;
                }
            }

            request.headers[nam] = val;
        }

        return true;
    }
}