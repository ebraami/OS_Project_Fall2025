#pragma once
#include <windows.h>

class Semaphore
{
public:
    Semaphore();
    void wait();
    void signal();

private:
    HANDLE sem;
};
