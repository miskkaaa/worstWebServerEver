#pragma once
class server {
    public:
    explicit server(int port);
    void run(); // sure

    private:
    int port;
    int server_fd;

    void handleClient(int id);
};