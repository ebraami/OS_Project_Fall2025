#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <cstdint>

class ChatClient
{
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    ChatClient();
    ~ChatClient();

    ChatClient(const ChatClient&) = delete;
    ChatClient& operator=(const ChatClient&) = delete;

    void setOnMessage(const MessageCallback& cb);
    void setOnStatus(const StatusCallback& cb);
    void setOnError(const ErrorCallback& cb);

    bool connectTo(const std::string& host, uint16_t port);
    void disconnect();
    bool isConnected() const;
    bool sendMessage(const std::string& message);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    MessageCallback onMessage_;
    StatusCallback onStatus_;
    ErrorCallback onError_;
};
