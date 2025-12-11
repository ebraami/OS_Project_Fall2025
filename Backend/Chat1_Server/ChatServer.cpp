#include "ChatServer.h"

ChatServer::ChatServer()
    : listenSocket(INVALID_SOCKET), isRunning(false)
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

    isRunning.store(true);
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void ChatServer::run()
{
    while (isRunning.load())
    {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET client = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);

        if (client == INVALID_SOCKET)
        {
            if (isRunning.load())
            {
                std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
            }
            break;
        }

        char ip[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        std::cout << "Accepted connection from " << ip << ":" << ntohs(clientAddr.sin_port) << " socket=" << client << std::endl;

        {
            std::lock_guard<std::mutex> lg(clientsMutex);
            clients.push_back(client);
        }

        clientThreads.emplace_back(&ChatServer::handleClient, this, client);
    }
}

void ChatServer::stop()
{
    if (!isRunning.exchange(false)) return;

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
    }

    for (auto& t : clientThreads)
    {
        if (t.joinable()) t.join();
    }

    clientThreads.clear();
    {
        std::lock_guard<std::mutex> lg(clientsMutex);
        clients.clear();
    }

    std::cout << "Server stopped." << std::endl;
}

void ChatServer::handleClient(SOCKET clientSocket)
{
    const int BUFSIZE = 1024;
    char buffer[BUFSIZE];

    while (isRunning.load())
    {
        int r = recv(clientSocket, buffer, BUFSIZE - 1, 0);
        if (r > 0)
        {
            buffer[r] = '\0';
            std::cout << "[recv from " << clientSocket << "] " << buffer << std::endl;
            broadcast(buffer, r, clientSocket);
        }
        else if (r == 0)
        {
            std::cout << "Client " << clientSocket << " disconnected." << std::endl;
            break;
        }
        else
        {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK || err == WSAEINTR)
                continue;
            std::cerr << "recv() error on socket " << clientSocket << ": " << err << std::endl;
            break;
        }
    }

    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    removeClient(clientSocket);
    std::cout << "Handler ended for socket " << clientSocket << std::endl;
}

void ChatServer::broadcast(const char* data, int len, SOCKET exceptSock)
{
    std::vector<SOCKET> copy;
    {
        std::lock_guard<std::mutex> lg(clientsMutex);
        copy = clients;
    }

    for (SOCKET s : copy)
    {
        if (s == exceptSock) continue;
        int sent = send(s, data, len, 0);
        if (sent == SOCKET_ERROR)
        {
            std::cerr << "send() failed to socket " << s << ": " << WSAGetLastError() << std::endl;
        }
    }
}

void ChatServer::removeClient(SOCKET s)
{
    std::lock_guard<std::mutex> lg(clientsMutex);
    for (auto it = clients.begin(); it != clients.end(); ++it)
    {
        if (*it == s)
        {
            clients.erase(it);
            break;
        }
    }
}
