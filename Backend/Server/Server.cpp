#include "Server.h"
#include <algorithm>

std::vector<SOCKET> Server::clients;
std::mutex Server::clientsMutex;
std::vector<std::thread> Server::threads;

Server::Server(int port) : port(port) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
    }
}

Server::~Server() {
    running = false;
    closesocket(serverSocket);
    WSACleanup();
}

void Server::start() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Server is listening on port " << port << "...\n";
    running = true;
    acceptClients();
}

void Server::acceptClients() {
    while (running) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
            std::cout << "New client connected! Total: " << clients.size() << "\n";
        }

        threads.emplace_back(handleClient, clientSocket);
    }
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[1024];

    while (true) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) {
            // Disconnect
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
                std::cout << "Client disconnected. Remaining: " << clients.size() << "\n";
            }
            closesocket(clientSocket);
            return;
        }

        buffer[bytes] = '\0';
        std::string message = "[Client " + std::to_string(clientSocket) + "]: " + buffer;

        // Broadcast
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (SOCKET other : clients) {
                if (other != clientSocket) {
                    send(other, message.c_str(), message.length(), 0);
                }
            }
        }
    }
}