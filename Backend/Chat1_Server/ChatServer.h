#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

class ChatServer
{
public:
    ChatServer();
    ~ChatServer();

    bool start(unsigned short port);
    void run();
    void stop();

private:
    SOCKET listenSocket;
    std::atomic<bool> isRunning;
    std::vector<SOCKET> clients;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;

    void handleClient(SOCKET clientSocket);
    void broadcast(const char* data, int len, SOCKET exceptSock);
    void removeClient(SOCKET s);
};
