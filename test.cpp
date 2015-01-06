#include <iostream>
#include <atomic>
#include <random>
#include <future>

#include "./tcp/tcp_server.h"

BrinK::tcp::server                   tcp_server__;

volatile std::atomic_bool            exit__;

void random_test_broadcast()
{
    static int send_count=0;

    while (!exit__)
    {
        tcp_server__.broadcast(BrinK::utils::to_string < int >(++send_count));
        BrinK::utils::sleep(BrinK::utils::random(100,200));
    }
}

void random_test_start_stop()
{
    while (!exit__)
    {
        tcp_server__.start(99);
        BrinK::utils::sleep(BrinK::utils::random(100, 200));
        tcp_server__.stop();
    }
}

int main(int, char**)
{
    unsigned int port=99;
    tcp_server__.start(port);

    std::cout << "Server started port : " << port << std::endl;

    do
    {
        std::string cmd;

        std::getline(std::cin, cmd);

        if (cmd == "q")
        {
            std::cout << "Stop server" << std::endl;

            std::async([]
            {
                tcp_server__.stop();
            });
        }

        else if (cmd == "s")
        {
            std::cout << "Start server" << std::endl;

            std::async([port]
            {
                tcp_server__.start(port);
            });

        }
        else if (cmd == "t")
        {
            std::cout << "Test thread start/stop" << std::endl;
            exit__ = false;
            std::async([]{random_test_start_stop(); });
        }
        else if (cmd.substr(0, 1) == "w")
        {
            std::cout << "Start broadcast thread" << std::endl;
            exit__ = false;
            std::async([]
            {
                random_test_broadcast();
            });
        }
        else if (cmd == "b")
        {
            std::cout << "Broadcast msg" << std::endl;

            std::async([]
            { 
                tcp_server__.broadcast("broadcast");
            });

        }
        else if (cmd == "qt")
        {
            std::cout << "Stop threads" << std::endl;

            exit__ = true;
        }
    } while (true);

    system("pause");

    return 0;
}