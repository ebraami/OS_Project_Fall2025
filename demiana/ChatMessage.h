#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <windows.h>

#define MAX_MESSAGE_LENGTH 256
#define SHARED_MEMORY_NAME L"ChatSharedMemory"


struct ChatMessage
{
    char message[MAX_MESSAGE_LENGTH];
    bool hasMessage;
};

bool createSharedMemory();
void writeMessage(const char* msg);
bool readMessage(char* buffer);
void detachSharedMemory();

#endif

