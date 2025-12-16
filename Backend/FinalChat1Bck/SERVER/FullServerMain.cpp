#include "CharServer.h"
#include <iostream>

int main()
{
    ChatServer server;

    if (!server.start(54000))
    {
        std::cout << "Failed to start server\n";
        return 1;
    }

    server.run(); 
    return 0;
}
