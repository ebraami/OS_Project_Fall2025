#include "ChatServer.h"
#include <windows.h>

static ChatServer* g_server = nullptr;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    if (g_server)
    {
        g_server->stop();
    }
    return TRUE;
}

int main()
{
    ChatServer server;
    g_server = &server;

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        std::cerr << "Could not set control handler" << std::endl;
    }

    if (!server.start(54000))
    {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    server.run();

    return 0;
}
