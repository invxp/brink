#include <process.h>
#include <string>
#include <vector>
#include <atomic>
#include <iostream>
#include <memory>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/any.hpp>

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
    std::string buf = "12345678901234567890";

    while (true)
    {
        if (g_exit)
            break;

        if (S == INVALID_SOCKET)
            S = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80);
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
        server_addr.sin_port = htons(80);
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

void thread_func_http_test()
{

    bool   conned = false;
    SOCKET S = INVALID_SOCKET;
    std::string buf = "GET / HTTP1.1\r\n\r\nCaoNiMa";
    while (true)
    {
        if (g_exit)
            break;

        if (S == INVALID_SOCKET)
            S = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80);
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

        send(S, buf.c_str(), buf.length(), 0);

        closesocket(S);
        S = INVALID_SOCKET;
        conned = false;
        Sleep(10);

    }
}

bool calc_file_date_expired(const std::wstring& file)
{
    HANDLE        h;
    FILETIME      cr;
    SYSTEMTIME    last;
    SYSTEMTIME    today;

    GetLocalTime(&today);

    h = CreateFile(file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (!h)
        return true;

    if (!GetFileTime(h, &cr, nullptr, nullptr))
    {
        CloseHandle(h);
        return true;
    }

    CloseHandle(h);

    if (!FileTimeToSystemTime(&cr, &last))
        return true;

    if ((today.wYear != last.wYear) || (today.wMonth != last.wMonth) || (today.wDay != last.wDay))
        return true;

    return false;
}



class test
{
public:
    test(){ std::cout << "Create" << std::endl; myfalg =false; }
    ~test(){ std::cout << "Release" << std::endl; if (myfalg) return; delete this; myfalg = true; }

public:
    int m;
    bool myfalg;
};


int main(int argc, char** argv)
{

    {
        test aaa;
        aaa.m = 1;
    }
    g_exit = false;
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    //    HANDLE H = CreateFileA("", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    std::string test1 = "C:\\Users\\InvXp\\AppData\\Roaming\\youku\\cache\\_aHR0cHM6Ly9vcGVuYXBpLnlvdWt1LmNvbS92Mi91c2Vycy9wbGF5bG9nL2dldC5qc29uP2NsaWVudF9pZD1lNTdiYzgyYjFhOWRjZDJmJnVzZXJfdHlwZT1ndWlkJnVzZXJfaWQ9MTEwMDAwMDAwMDAwMDAwMDAwMDA1NDc1NEU2MDI4RDI0NDU1NjU3MyZzdGFydF90aW1lPTAmcGFnZT0xJmNvdW50PTEwMA==";

    size_t len = test1.length();

    if (len>=256)
        test1=test1.substr(0, 259);

    HANDLE H = CreateFileA(test1.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (H == INVALID_HANDLE_VALUE)
    {
        DWORD DW = GetLastError();
        int a=0;
    }
    /*
    for (int i = 0; i < 100; i++)
        boost::thread thread(&thread_func_asio_test);

    for (int i = 0; i < 100; i++)
        boost::thread thread(&thread_func_only_conn);

    for (int i = 0; i < 100; i++)
        boost::thread thread(&thread_func_http_test);
    */
    thread_func_http_test();

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