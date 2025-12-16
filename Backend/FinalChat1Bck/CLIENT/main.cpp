#include "ChatClient.h"
#include <iostream>
#include <string>

int main()
{
    ChatClient client;

    client.setOnStatus([](const std::string& s)
        {
            std::cout << "[STATUS] " << s << std::endl;
        });

    client.setOnMessage([](const std::string& m)
        {
            std::cout << "[RECEIVED] " << m << std::endl;
        });

    client.setOnError([](const std::string& e)
        {
            std::cout << "[ERROR] " << e << std::endl;
        });

    if (!client.connectTo("127.0.0.1", 54000))
    {
        std::cout << "Could not connect to server\n";
        return 1;
    }

    std::cout << "Type messages. Type 'exit' to quit.\n";

    std::string input;
    while (true)
    {
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        client.sendMessage(input);
    }

    client.disconnect();
    return 0;
}
