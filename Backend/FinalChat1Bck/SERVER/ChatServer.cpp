#include "CharServer.h"
#include <algorithm>

ChatServer::ChatServer()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

ChatServer::~ChatServer()
{
    stop();
    WSACleanup();
}

bool ChatServer::start(unsigned short port)
{
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return false;

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return false;

    running = true;
    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

void ChatServer::run()
{
    while (running)
    {
        SOCKET client = accept(listenSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(client);
            std::cout << "Client connected. Total: " << clients.size() << std::endl;
        }

        threads.emplace_back(&ChatServer::handleClient, this, client);
    }
}

void ChatServer::handleClient(SOCKET client)
{
    char buffer[1024];

    while (true)
    {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::string msg = buffer;

        std::cout << "Received: " << msg << std::endl;
        broadcast(msg, client);
    }

    closesocket(client);

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
        std::cout << "Client disconnected. Remaining: " << clients.size() << std::endl;
    }
}

void ChatServer::broadcast(const std::string& msg, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (SOCKET c : clients)
    {
        if (c != sender)
            send(c, msg.c_str(), (int)msg.size(), 0);
    }
}

void ChatServer::stop()
{
    running = false;
    closesocket(listenSocket);

    for (auto& t : threads)
        if (t.joinable()) t.join();
}
