#include <iostream>
#include <atomic>
#include <random>
#include <future>

#include "./tcp/tcp_server.h"

BrinK::tcp::server                   tcp_server__;

volatile std::atomic_bool            exit__;

void random_test_broadcast()
{
    std::mt19937 engine(std::time(nullptr));
    std::uniform_int_distribution < int > num(100, 1000);

    static int send_count=0;

    while (!exit__)
    {
        int rand = num(engine);
        tcp_server__.broadcast(BrinK::utils::to_string < int >(++send_count));
        std::this_thread::sleep_for(std::chrono::milliseconds(rand));
    }

}

void random_test_start_stop()
{
    std::mt19937 rng(std::time(nullptr));
    std::uniform_int<> ui(1000, 2000);

    while (!exit__)
    {
        int rand = ui(rng);
        tcp_server__.start(99);
        std::this_thread::sleep_for(std::chrono::milliseconds(rand));
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