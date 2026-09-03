#include "includes/server.hpp"
#include <iostream>
int main() {
    try {
        server server(6767);
        server.run();
        std::cout << "hi from server im running at port 6767\n";
    } catch (const std::exception& err) {
        std::cerr << "error: " << err.what() << "\n";
        return 1;
    }

    return 0;
}