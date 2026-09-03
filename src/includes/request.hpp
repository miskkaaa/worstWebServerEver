#pragma once
#include <string>
#include <unordered_map>
namespace http {
    struct Request {
        std::string method;
        std::string path;
        std::string version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
    };

    bool parse(const std::string& raw, Request& request);
};