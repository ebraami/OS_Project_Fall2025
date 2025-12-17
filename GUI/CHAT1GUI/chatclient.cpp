#include "ChatClient.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

class ChatClient::Impl
{
public:
    SOCKET socket = INVALID_SOCKET;
    std::thread recvThread;
    std::atomic<bool> connected{ false };
};

ChatClient::ChatClient()
    : impl_(std::make_unique<Impl>())
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    onMessage_ = [](const std::string&) {};
    onStatus_ = [](const std::string&) {};
    onError_ = [](const std::string&) {};
}

ChatClient::~ChatClient()
{
    disconnect();
    WSACleanup();
}

void ChatClient::setOnMessage(const MessageCallback& cb) { onMessage_ = cb; }
void ChatClient::setOnStatus(const StatusCallback& cb) { onStatus_ = cb; }
void ChatClient::setOnError(const ErrorCallback& cb) { onError_ = cb; }

bool ChatClient::connectTo(const std::string& host, uint16_t port)
{
    impl_->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (impl_->socket == INVALID_SOCKET)
    {
        onError_("Socket creation failed");
        return false;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server.sin_addr);

    if (connect(impl_->socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        onError_("Connection failed");
        closesocket(impl_->socket);
        return false;
    }

    impl_->connected = true;
    onStatus_("Connected to server");

    impl_->recvThread = std::thread([this]()
        {
            char buffer[1024];
            while (impl_->connected)
            {
                int bytes = recv(impl_->socket, buffer, sizeof(buffer) - 1, 0);
                if (bytes <= 0) break;
                buffer[bytes] = '\0';
                onMessage_(buffer);
            }
            impl_->connected = false;
            onStatus_("Disconnected from server");
        });

    return true;
}

void ChatClient::disconnect()
{
    if (!impl_->connected) return;

    impl_->connected = false;
    shutdown(impl_->socket, SD_BOTH);
    closesocket(impl_->socket);

    if (impl_->recvThread.joinable())
        impl_->recvThread.join();
}

bool ChatClient::isConnected() const
{
    return impl_->connected;
}

bool ChatClient::sendMessage(const std::string& message)
{
    if (!impl_->connected) return false;

    return send(
        impl_->socket,
        message.c_str(),
        static_cast<int>(message.size()),
        0
    ) != SOCKET_ERROR;
}
