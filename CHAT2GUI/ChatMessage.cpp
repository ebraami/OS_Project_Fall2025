#include "ChatMessage.h"
#include <windows.h>
#include <iostream>

ChatMessage* createSharedMemory()
{
    HANDLE hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(ChatMessage),
        L"Chat2SharedMemory"
    );

    if (!hMap)
        return nullptr;

    ChatMessage* msg = (ChatMessage*)MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(ChatMessage)
    );

    return msg;
}
