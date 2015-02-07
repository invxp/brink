#include <iostream>
#include <atomic>
#include <random>
#include <future>
#include <pool/thread.hpp>

#include "./tcp/tcp_server.h"
#include "./tcp/protobuf/protobuf_server.h"
#include "./tcp/http/http_server.h"

BrinK::tcp::http_server             http_server;
std::atomic_bool                    sig_exit;

#include <pool/shared.hpp>

void random_test_broadcast()
{
    static int send_count=0;

    while (!sig_exit)
    {
        http_server.broadcast("Broadcast!!!");
        BrinK::utils::sleep(BrinK::utils::random(100,200));
    }
}

void random_test_start_stop()
{
    while (!sig_exit)
    {
        http_server.start(80);
        BrinK::utils::sleep(BrinK::utils::random(100, 200));
        http_server.stop();
    }
}

int main(int, char**)
{
//     unsigned long test_count = 1000000000;
// 
//     BrinK::pool::pool< std::string > pool([&test_count]{return BrinK::utils::to_string<unsigned long>(++test_count); }, 55);
// 
//     {
//         for (;;)
//         {
//             std::string s;
//             pool.get([&s](const std::string& str)
//             {
//                 s = str;
//             });
//             pool.free(s);
// 
//         }
//     }
    unsigned int port=80;
    http_server.start(port);

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
                http_server.stop();
            });
        }
        else if (cmd == "s")
        {
            std::cout << "Start server" << std::endl;

            std::async([port]
            {
                http_server.start(port);
            });

        }
        else if (cmd == "t")
        {
            std::cout << "Test thread start/stop" << std::endl;
            sig_exit = false;
            std::async([]{random_test_start_stop(); });
        }
        else if (cmd.substr(0, 1) == "w")
        {
            std::cout << "Start broadcast thread" << std::endl;
            sig_exit = false;
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
                http_server.broadcast("A test");
            });

        }
        else if (cmd == "p")
        {
            std::async([]
            {
                std::cout << "Test Functions" << std::endl;

            });
        }
        else if (cmd == "qt")
        {
            std::cout << "Stop threads" << std::endl;

            sig_exit = true;
        }
    } while (true);

    system("pause");

    return 0;
}