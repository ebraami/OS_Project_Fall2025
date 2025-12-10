#pragma once
#include <winsock2.h> // SOCKET => socket(), bind(), listen(), accept(), send(), recv(), closesocket()
#include <ws2tcpip.h> // ipv6
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

class Server {
public:
    Server(int port = 8080);
    ~Server();
    void start();

private:
    void acceptClients();
    static void handleClient(SOCKET clientSocket);

    int port;
    SOCKET serverSocket;
    bool running = false;

    // Shared between all threads
    static std::vector<SOCKET> clients;
    static std::mutex clientsMutex;
    static std::vector<std::thread> threads;
};