#include <Windows.h>
#include <WinSock.h>
#include <process.h>
#include <string>
#include <vector>
#include <atomic>
#include <iostream>
#include <boost/thread.hpp>

using namespace std;

#pragma comment (lib,"ws2_32.lib")

volatile std::atomic_bool g_exit;

void thread_func_asio_test();
void thread_func_for_utest();
void thread_func_only_conn();

void beep_start()
{
    Beep(262, 230);
    Beep(294, 230);
    Beep(330, 230);
    Beep(349, 230);
    Beep(392, 230);
    Beep(440, 230);
    Beep(494, 550);

    Beep(494, 230);
    Beep(440, 230);
    Beep(392, 230);
    Beep(349, 230);
    Beep(330, 230);
    Beep(294, 230);
    Beep(262, 530);
}


void thread_func_for_utest()
{
    int conned = 0;

    SOCKET S = INVALID_SOCKET;
    while (true)
    {
        int udp_send = 0;

        if (g_exit)
            break;

        S = socket(AF_INET, SOCK_DGRAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(99);

        udp_send = sendto(S, "12345", 5, 0, (sockaddr*)&server_addr, sizeof(sockaddr_in));

        DWORD dw = GetLastError();

        if (udp_send <= 0)
        {
            printf("error %ld , %d \n", dw, conned);
        }
        else
        {
            conned++;
        }
        Sleep(10);
    }

}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

        switch (wParam)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {

            if (p->vkCode == VK_F10 && GetKeyState(VK_CONTROL) & 0x8000)
            {

            }

            if (p->vkCode == VK_F11 && GetKeyState(VK_CONTROL) & 0x8000)
            {

            }

            if (p->vkCode == VK_F12 && GetKeyState(VK_CONTROL) & 0x8000)
            {

            }

            if (p->vkCode == VK_SCROLL)
            {

            }

            break;
        }
        default:
            break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void thread_func_asio_test()
{
    bool   conned = false;
    SOCKET S = INVALID_SOCKET;
    std::string buf = "TEST";

    while (true)
    {
        if (g_exit)
            break;

        if (S == INVALID_SOCKET)
            S = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(99);
        if (!conned)
        {
            if (connect(S, (struct sockaddr*)&server_addr, sizeof(server_addr)))
            {
                closesocket(S);
                S = INVALID_SOCKET;
                Sleep(10);
                continue;
            }
            conned = true;
        }

        int sent = send(S, buf.c_str(), buf.length(), 0);
        if (sent <= 0)
        {
            closesocket(S);
            S = INVALID_SOCKET;
            conned = false;
            Sleep(10);
            continue;
        }

        std::cout << "Sent OK!" << std::endl;

        char recvbuf[1024] = { 0 };

        int received = recv(S, recvbuf, 1024, 0);

        if (received <= 0)
        {
            closesocket(S);
            S = INVALID_SOCKET;
            conned = false;
            Sleep(10);
            continue;
        }
        else
        {
            std::cout << "Recv £º " << recvbuf << std::endl;
        }

        Sleep(10);
    }
}

void thread_func_only_conn()
{
    bool   conned = false;
    SOCKET S = INVALID_SOCKET;
    static unsigned __int64 conned_count = 0;

    while (true)
    {
        if (g_exit)
            break;

        if (S == INVALID_SOCKET)
            S = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(99);
        if (!conned)
        {
            if (connect(S, (struct sockaddr*)&server_addr, sizeof(server_addr)))
            {
                closesocket(S);
                S = INVALID_SOCKET;
                Sleep(10);
                continue;
            }
            conned = true;
            char buf[1024] = { 0 };
            for (;;)
            {
                if (recv(S, buf, 1024, 0) <= 0)
                    break;
                else
                    Sleep(10);
            }

            printf("Only Conn : %d\n", ++conned_count);
            closesocket(S);
            conned = false;
            S = INVALID_SOCKET;
            Sleep(10);
            continue;
        }

    }
}
int main(int argc, char** argv)
{
    g_exit = false;
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
    for (int i = 0; i < 100; i++)
        boost::thread thread(&thread_func_asio_test);
 
    for (int i = 0; i < 100; i++)
        boost::thread thread(&thread_func_only_conn);

    //thread_func_only_conn();

    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

    while (!g_exit)
    {
        MSG msg;
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    UnhookWindowsHookEx(hhkLowLevelKybd);
    return 0;
}