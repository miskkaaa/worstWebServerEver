#include "includes/server.hpp"
#include "includes/request.hpp"
#include <iostream>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::string res(const std::string& stat, const std::string& type, const std::string& body) {
    return
        "HTTP/1.1 " + stat + "\r\n"
        "Content-Type: " + type + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;
}

std::string file(const std::string& path) {
    std::filesystem::path root = "../../src/htdocs";

    if (path == "/") {
        root /= "index.html";
    } else {
        root /= path.substr(1);
    }
    
    std::filesystem::path filePath = std::filesystem::weakly_canonical(root);
    std::filesystem::path basePath = std::filesystem::weakly_canonical("../../src/htdocs");
    auto base = basePath.begin();
    auto current = filePath.begin();

    for (; base != basePath.end(); ++base, ++current) {
        if (current == filePath.end() || *base != *current) {
            return "";
        }
    }

    if (!std::filesystem::exists(filePath) ||
        !std::filesystem::is_regular_file(filePath)) {
        return "";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file) return "";

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string content(const std::string& path) {
    std::filesystem::path filePath(path);
    std::string ext = filePath.extension().string();

    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".txt") return "text/plain";

    return "text/html";
}


server::server(int port) : port(port), server_fd(-1), config(cfg::loadConfig("../../src/htdocs/.wsecfg")) {
    std::cout << "404 page: " << config.page404 << "\n";
    std::cout << "400 page: " << config.page400 << "\n";

    for (const auto& file : config.hiddenFiles) {
        std::cout << "hidden file: " << file << "\n";
    }

    for (const auto& folder : config.hiddenFolders) {
        std::cout << "hidden folder: " << folder << "\n";
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) throw std::runtime_error("failed to create socket");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server_fd);
        throw std::runtime_error("failed to create socket");
    }

    sockaddr_in adress{};                       // configure adress
    adress.sin_family = AF_INET;                // inet thingy idk the tutorial doesnt explain it
    adress.sin_addr.s_addr = INADDR_ANY;        // all network interfaces
    adress.sin_port = htons(port);    // convert port to byte order

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&adress), sizeof(adress)) == -1) {
        close(server_fd);
        throw std::runtime_error("failed to bind");
    }

    if (listen(server_fd, 10) == -1) {
        close(server_fd);
        throw std::runtime_error("failed to listen");
    }

    std::cout << "hi im on " << port << "\n";
}

void server::run() {
    while (true) {
        sockaddr_in adress{};
        socklen_t length = sizeof(adress);

        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&adress), &length);

        if (client_fd == -1) {
            std::cerr << "failed to accept client\n";
            continue;
        }

        handleClient(client_fd);
        close(client_fd);
    }
}

void server::handleClient(int fd) {
    char buf[8192];
    const ssize_t bread = recv(fd, buf, sizeof(buf) - 1, 0);
    if (bread <= 0) return;

    buf[bread] = '\0';
    std::string rawreq(buf);

    std::cout << "raw request\n";

    http::Request request;

    /* 400 */
    if (!http::parse(rawreq, request)) {
        std::string body = file(config.page400);

        if (body.empty()) {
            body = "<h1>400 Bad Request</h1>";
        }

        const std::string response = res(
            "400 Bad Request",
            "text/html",
            body
        );

        send(fd, response.c_str(), response.size(), 0);

        return;
    }

    std::cout << "Method: " << request.method << "\n";
    std::cout << "Path: " << request.path << "\n";

    /* Routing */
    std::string status;
    std::string body;

    if (request.method != "GET") {
        status = "405 Method Not Allowed";
        body = "<title>405</title><body><h1>405 Method Not Allowed</h1></body>";
    } else if (cfg::isHiddenFile(request.path, config) ||
               cfg::isHiddenFolder(request.path, config)) {

        status = "404 Not Found";
        body = file(config.page404);

        if (body.empty()) {
            body = "<h1>404 Not Found</h1>";
        }
    } else if (request.path == "/api/test") {
        status = "200 OK";
        body = 
            "{"
            "\"message\":\"bullshit 123\""
            "}";
    } else {
        body = file(request.path);

        if (body.empty()) {
            status = "404 Not Found";
            body = file(config.page404);

            if (body.empty()) {
                body = "<h1>404 Not Found</h1>";
            }
        } else {
            status = "200 OK";
        }
    }

    std::string type = content(request.path);

    const std::string response = res(status, type, body);
    send(fd, response.c_str(), response.size(), 0);
}