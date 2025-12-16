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
        if (msg->hasBtoA)
        {
            std::cout << "\nB: " << msg->msgBtoA << std::endl;
            msg->hasBtoA = false;
            std::cout << "A: ";
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
        strncpy_s(msg->msgAtoB, input.c_str(), sizeof(msg->msgAtoB));
        msg->hasAtoB = true;
        sem.signal();
    }
}

int main()
{
    msg = createSharedMemory();
    if (!msg) return 1;

    msg->hasAtoB = false;
    msg->hasBtoA = false;

    std::cout << "Process A started\nA: ";

    std::thread r(readerThread);
    std::thread w(writerThread);

    r.join();
    w.join();
}
