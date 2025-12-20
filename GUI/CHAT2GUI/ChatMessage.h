#pragma once

struct ChatMessage
{
    char msgAtoB[256];
    char msgBtoA[256];
    bool hasAtoB;
    bool hasBtoA;
};

ChatMessage* createSharedMemory();
