#include <iostream>
#include <atomic>
#include <random>
#include <future>
#include <pool/thread.hpp>

#include "./tcp/tcp_server.h"
#include "./tcp/protobuf/protobuf_server.h"

BrinK::tcp::server                  server;
std::atomic_bool                    sig_exit;

#include <pool/shared.hpp>

void random_test_broadcast()
{
    static int send_count=0;

    while (!sig_exit)
    {
        server.broadcast("Broadcast!!!");
        BrinK::utils::sleep(BrinK::utils::random(100,200));
    }
}

void random_test_start_stop()
{
    while (!sig_exit)
    {
        server.start(80);
        BrinK::utils::sleep(BrinK::utils::random(100, 200));
        server.stop();
    }
}

int main(int, char**)
{
    unsigned int port=80;
    server.start(port);

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
                server.stop();
            });
        }
        else if (cmd == "s")
        {
            std::cout << "Start server" << std::endl;

            std::async([port]
            {
                server.start(port);
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
                server.broadcast("A test");
            });

        }
        else if (cmd == "p")
        {
            std::async([]
            {

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