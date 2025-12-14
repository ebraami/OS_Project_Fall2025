#include <windows.h>
#include <string>
#include <vector>
#include <thread>

#define SHM_NAME  "Global\\MyChatMemory"
#define MUTEX_NAME "Global\\MyChatMutex"
#define EVENT_NAME "Global\\MyChatEvent"

#define MAX_MESSAGES 32
#define MSG_SIZE 256

struct ShmData {
    unsigned long long seq;
    char msgs[MAX_MESSAGES][MSG_SIZE];
};

// global variables
HWND hEditInput, hListBox, hButtonSend;
HANDLE hMapFile, hMutex, hEvent;
ShmData* shm;
unsigned long long lastSeq = 0;
bool running = true;

std::string username;

//  username window
HWND hNameEdit;
bool gotUsername = false;

LRESULT CALLBACK NameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // button ok
            char buf[64];
            GetWindowTextA(hNameEdit, buf, 64);
            username = buf;
            if (username.empty()) username = "Client";
            gotUsername = true;
            DestroyWindow(hwnd);
        }
        break;
    case WM_CLOSE:
        if (!gotUsername) username = "Client";
        gotUsername = true;
        DestroyWindow(hwnd);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void UsernameThread(HINSTANCE hInst) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = NameWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "NameWnd";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("NameWnd", "Enter Username",
        WS_OVERLAPPEDWINDOW, 500, 300, 300, 120,
        NULL, NULL, hInst, NULL);

    hNameEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
        20, 20, 240, 25, hwnd, NULL, hInst, NULL);

    CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        90, 60, 100, 25, hwnd, (HMENU)1, hInst, NULL);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (!gotUsername) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }
}

// add to listbox
void AddToListBox(const char* msg) {
    SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)msg);
}

//  receiver thread
DWORD WINAPI ReceiverThread(LPVOID) {
    while (running) {
        WaitForSingleObject(hEvent, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        while (lastSeq < shm->seq) {
            lastSeq++;
            unsigned index = lastSeq % MAX_MESSAGES;
            AddToListBox(shm->msgs[index]);
        }

        ReleaseMutex(hMutex);
    }
    return 0;
}

//  send message
void SendMessageToServer() {
    char text[MSG_SIZE];
    GetWindowTextA(hEditInput, text, MSG_SIZE);

    if (strlen(text) == 0) return;

    WaitForSingleObject(hMutex, INFINITE);

    shm->seq++;
    unsigned index = shm->seq % MAX_MESSAGES;

    std::string msg = username + ": " + text;
    size_t len = msg.size();
    if (len >= MSG_SIZE) len = MSG_SIZE - 1;

    memcpy(shm->msgs[index], msg.c_str(), len);
    shm->msgs[index][len] = '\0';

    ReleaseMutex(hMutex);
    SetEvent(hEvent);

    SetWindowTextA(hEditInput, "");
}

//  main window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // send button
            SendMessageToServer();
        }
        break;
    case WM_DESTROY:
        running = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//  winmain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    //  ask for username
    std::thread t(UsernameThread, hInstance);
    while (!gotUsername) Sleep(10);
    t.join();

    //  Create Shared Memory
    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        sizeof(ShmData), SHM_NAME);
    shm = (ShmData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ShmData));
    hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    hEvent = CreateEventA(NULL, TRUE, FALSE, EVENT_NAME);

    // main gui
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChatClientWnd";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("ChatClientWnd", "Shared Memory Client",
        WS_OVERLAPPEDWINDOW, 200, 200, 500, 400,
        NULL, NULL, hInstance, NULL);

    hEditInput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
        20, 20, 300, 25, hwnd, NULL, hInstance, NULL);

    hButtonSend = CreateWindowA("BUTTON", "Send",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        330, 20, 100, 25, hwnd, (HMENU)1, hInstance, NULL);

    hListBox = CreateWindowA("LISTBOX", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
        20, 60, 440, 280, hwnd, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    //  start receiver thread
    CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);

    //  message loop
    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    running = false;
    return 0;
}
