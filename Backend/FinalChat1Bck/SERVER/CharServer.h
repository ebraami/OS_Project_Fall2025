#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

class ChatServer
{
public:
    ChatServer();
    ~ChatServer();

    bool start(unsigned short port);
    void run();   // blocking loop
    void stop();

private:
    void handleClient(SOCKET clientSocket);
    void broadcast(const std::string& message, SOCKET sender);

    SOCKET listenSocket = INVALID_SOCKET;
    std::atomic<bool> running{ false };

    std::vector<SOCKET> clients;
    std::vector<std::thread> threads;
    std::mutex clientsMutex;
};
