#include "ChatServer.h"
#include <algorithm>

ChatServer::ChatServer()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        throw std::runtime_error("WSAStartup failed");
    }
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

    // SO_REUSEADDR عشان الـ port يتفك بسرعة بعد الإغلاق
    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return false;
    }

    isRunning = true;
    std::cout << "Chat Server started on port " << port << " (Ctrl+C to stop)\n";
    return true;
}

void ChatServer::run()
{
    while (isRunning)
    {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET client = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);

        if (client == INVALID_SOCKET)
        {
            if (isRunning)
                std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        char ip[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        std::cout << "New client connected: " << ip << ":" << ntohs(clientAddr.sin_port)
                  << " (socket=" << client << ") Total clients: " << (clients.size() + 1) << "\n";

        {
            std::lock_guard<std::mutex> lg(clientsMutex);
            clients.push_back(client);
        }

        clientThreads.emplace_back(&ChatServer::handleClient, this, client);
    }
}

void ChatServer::handleClient(SOCKET clientSocket)
{
    char buffer[1024];
    while (isRunning)
    {
        int r = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (r > 0)
        {
            buffer[r] = '\0';
            std::string msg = "[Socket " + std::to_string(clientSocket) + "]: " + buffer;
            std::cout << msg << std::endl;
            broadcast(msg, clientSocket);
        }
        else if (r == 0)
        {
            std::cout << "Client " << clientSocket << " disconnected gracefully.\n";
            break;
        }
        else
        {
            std::cerr << "recv() error on " << clientSocket << ": " << WSAGetLastError() << std::endl;
            break;
        }
    }

    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    removeClient(clientSocket);
}

void ChatServer::broadcast(const std::string& message, SOCKET exceptSock)
{
    std::vector<SOCKET> toRemove;
    {
        std::lock_guard<std::mutex> lg(clientsMutex);
        for (SOCKET s : clients)
        {
            if (s == exceptSock) continue;

            if (send(s, message.c_str(), message.length(), 0) == SOCKET_ERROR)
            {
                std::cerr << "send() failed to " << s << ": " << WSAGetLastError() << std::endl;
                toRemove.push_back(s);
            }
        }
    }

    // Cleanup bad clients
    for (SOCKET bad : toRemove)
    {
        shutdown(bad, SD_BOTH);
        closesocket(bad);
        removeClient(bad);
        std::cout << "Removed disconnected client during broadcast: " << bad << "\n";
    }
}

void ChatServer::removeClient(SOCKET s)
{
    std::lock_guard<std::mutex> lg(clientsMutex);
    clients.erase(std::remove(clients.begin(), clients.end(), s), clients.end());
}

void ChatServer::stop()
{
    if (!isRunning.exchange(false)) return;

    std::cout << "\nShutting down server gracefully...\n";

    if (listenSocket != INVALID_SOCKET)
    {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    {
        std::lock_guard<std::mutex> lg(clientsMutex);
        for (SOCKET s : clients)
        {
            shutdown(s, SD_BOTH);
            closesocket(s);
        }
        clients.clear();
    }

    for (auto& t : clientThreads)
    {
        if (t.joinable()) t.join();
    }
    clientThreads.clear();

    std::cout << "Server stopped. All clients disconnected.\n";
}