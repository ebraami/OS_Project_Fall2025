#include "Semaphore.h"

Semaphore::Semaphore()
{
    sem = CreateSemaphore(
        NULL,
        1,
        1,
        L"Chat2Semaphore"
    );
}

void Semaphore::wait()
{
    WaitForSingleObject(sem, INFINITE);
}

void Semaphore::signal()
{
    ReleaseSemaphore(sem, 1, NULL);
}
