#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

class ChatServer
{
public:
    ChatServer();
    ~ChatServer();

    bool start(unsigned short port = 54000);
    void run();   // blocking
    void stop();

private:
    void handleClient(SOCKET clientSocket);
    void broadcast(const std::string& message, SOCKET exceptSock);

    void removeClient(SOCKET s);

    SOCKET listenSocket = INVALID_SOCKET;
    std::atomic<bool> isRunning{false};

    std::vector<SOCKET> clients;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;
};