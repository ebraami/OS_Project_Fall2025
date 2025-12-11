#include "Server.h"

int main() {
    try {
        Server server(8080);
        server.start();

        std::cout << "Press Enter to shut down the server...\n";
        std::cin.get();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}