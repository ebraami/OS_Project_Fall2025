#include "ChatMessage.h"
#include "Semaphore.h"
#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include <thread>

ChatMessage* msg;
Semaphore sem;

void readerThread()
{
    while (true)
    {
        sem.wait();
        if (msg->hasAtoB)
        {
            std::cout << "\nA: " << msg->msgAtoB << std::endl;
            msg->hasAtoB = false;
            std::cout << "B: ";
            std::cout.flush();
        }
        sem.signal();
        Sleep(200);
    }
}

void writerThread()
{
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);

        sem.wait();
        strncpy_s(msg->msgBtoA, input.c_str(), sizeof(msg->msgBtoA));
        msg->hasBtoA = true;
        sem.signal();
    }
}

int main()
{
    msg = createSharedMemory();
    if (!msg) return 1;

    std::cout << "Process B started\nB: ";

    std::thread r(readerThread);
    std::thread w(writerThread);

    r.join();
    w.join();
}
