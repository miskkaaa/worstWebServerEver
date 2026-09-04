#pragma once
#include "wsecfg.hpp"
class server {
    public:
        explicit server(int port);
        void run(); // sure

    private:
        int port;
        int server_fd;
        cfg::Config config;

        void handleClient(int id);
};