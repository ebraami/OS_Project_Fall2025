#include <windows.h>
#include <string>
#include <cstdio>

#define SHM_NAME  "Global\\MyChatMemory"
#define MUTEX_NAME "Global\\MyChatMutex"
#define EVENT_NAME "Global\\MyChatEvent"

#define MAX_MESSAGES 32
#define MSG_SIZE 256

struct ShmData {
    unsigned long long seq;
    char msgs[MAX_MESSAGES][MSG_SIZE];
};

HWND hEditInput = NULL, hListBox = NULL, hButtonSend = NULL;
HANDLE hMapFile = NULL, hMutex = NULL, hEvent = NULL;
ShmData* shm = nullptr;
unsigned long long lastSeq = 0;
bool running = true;
HANDLE hReceiverThread = NULL;

void AddToListBox(const char* msg) {
    if (hListBox) SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)msg);
}

DWORD WINAPI ReceiverThread(LPVOID) {
    while (running) {
        WaitForSingleObject(hEvent, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        while (lastSeq < shm->seq) {
            lastSeq++;
            unsigned index = (unsigned)(lastSeq % MAX_MESSAGES);
            AddToListBox(shm->msgs[index]);
        }

        ReleaseMutex(hMutex);
        ResetEvent(hEvent);
    }
    return 0;
}

void SendServerMessage() {
    char text[MSG_SIZE] = {0};
    GetWindowTextA(hEditInput, text, MSG_SIZE);
    if (strlen(text) == 0) return;

    WaitForSingleObject(hMutex, INFINITE);

    shm->seq++;
    unsigned index = (unsigned)(shm->seq % MAX_MESSAGES);
    std::string msg = std::string("Server: ") + text;
    size_t len = msg.size();
    if (len >= MSG_SIZE) len = MSG_SIZE - 1;
    memcpy(shm->msgs[index], msg.c_str(), len);
    shm->msgs[index][len] = '\0';

    ReleaseMutex(hMutex);
    SetEvent(hEvent);

    SetWindowTextA(hEditInput, "");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) SendServerMessage();
        break;
    case WM_DESTROY:
        running = false;
        if (hReceiverThread) { WaitForSingleObject(hReceiverThread, 500); CloseHandle(hReceiverThread); hReceiverThread = NULL; }
        if (shm) UnmapViewOfFile(shm);
        if (hMapFile) CloseHandle(hMapFile);
        if (hMutex) CloseHandle(hMutex);
        if (hEvent) CloseHandle(hEvent);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ShmData), SHM_NAME);
    if (!hMapFile) { MessageBoxA(NULL, "CreateFileMapping failed", "Error", MB_OK | MB_ICONERROR); return 1; }

    shm = (ShmData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ShmData));
    if (!shm) { MessageBoxA(NULL, "MapViewOfFile failed", "Error", MB_OK | MB_ICONERROR); CloseHandle(hMapFile); return 1; }

    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        shm->seq = 0;
        for (int i = 0; i < MAX_MESSAGES; ++i) shm->msgs[i][0] = '\0';
    }

    hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    hEvent = CreateEventA(NULL, TRUE, FALSE, EVENT_NAME);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChatServerWnd";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("ChatServerWnd", "Shared Memory Server", WS_OVERLAPPEDWINDOW, 200, 200, 500, 400, NULL, NULL, hInstance, NULL);

    hEditInput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 20, 300, 25, hwnd, NULL, hInstance, NULL);
    hButtonSend = CreateWindowA("BUTTON", "Broadcast", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 330, 20, 120, 25, hwnd, (HMENU)1, hInstance, NULL);
    hListBox = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY, 20, 60, 440, 280, hwnd, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    hReceiverThread = CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}

