#include "ChatMessage.h"
#include <iostream>

HANDLE hMapFile = NULL;
ChatMessage* sharedMsg = NULL;

bool createSharedMemory()
{
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(ChatMessage),
        SHARED_MEMORY_NAME
    );

    if (hMapFile == NULL)
        return false;

    sharedMsg = (ChatMessage*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(ChatMessage)
    );

    if (sharedMsg == NULL)
        return false;

    sharedMsg->hasMessage = false;
    return true;
}

void writeMessage(const char* msg)
{
    strcpy_s(sharedMsg->message, MAX_MESSAGE_LENGTH, msg);
    sharedMsg->hasMessage = true;
}

bool readMessage(char* buffer)
{
    if (!sharedMsg->hasMessage)
        return false;

    strcpy_s(buffer, MAX_MESSAGE_LENGTH, sharedMsg->message);
    sharedMsg->hasMessage = false;
    return true;
}

void detachSharedMemory()
{
    if (sharedMsg)
        UnmapViewOfFile(sharedMsg);

    if (hMapFile)
        CloseHandle(hMapFile);
}
