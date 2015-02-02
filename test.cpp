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

class test
{
public:
    test():v(998),k(1024),haha("haha"){ std::cout << "Create£º" << this << std::endl; }
    ~test(){ std::cout << "Release£º" << this << std::endl; }

    void a(){ std::cout << "Value£º" << ++v <<std::endl;}

    int v;
    int k;
    std::string haha;
};

void lazy(std::shared_ptr<test> p)
{
    static unsigned int count = 0;
    if (count > 50)
        return;

    BrinK::utils::sleep(10);

    std::async(std::bind(&lazy, p));

    ++count;

    p->a();
}

#include <pool/shared.hpp>

int main(int, char**)
{
//     {
//         BrinK::pool::shared<test> pool(2);
// 
//         pool.get([](std::shared_ptr<test> p)
//         {
//            std::async(std::bind(&lazy,p));
//         });
//     }
    
    BrinK::pool::shared<test> pp;

    BrinK::buffer buffer;

    buffer = "12345";

    buffer.get(4, 1, [](char* b, const size_t& count)
    {
        std::string buf(b, count);

        std::cout << buf << buf.length() << std::endl;
    });

    unsigned int port=80;
    http_server.start();

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
        else if (cmd == "qt")
        {
            std::cout << "Stop threads" << std::endl;

            sig_exit = true;
        }
    } while (true);

    system("pause");

    return 0;
}