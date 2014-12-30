#ifndef BRINK_UDP_SERVER_H
#define BRINK_UDP_SERVER_H

#include <brink_defines.h>
#include <atomic>
#include <mutex>

#include "udp_socket.h"

namespace BrinK
{
    namespace udp
    {
        typedef std::shared_ptr < BrinK::udp::socket > udp_client_sptr_t;

        class server :public boost::noncopyable
        {
        public:
            server();
            virtual ~server();

        public:
            virtual bool start(const unsigned int& port);
            virtual void stop();

        public:
            virtual unsigned int get_port();

        protected:
            virtual void start_();
            virtual void stop_();

        private:
            io_service_sptr_t                                       udp_io_service_;
            udp_client_sptr_t                                       udp_host_;
            work_sptr_t                                             udp_work_;
            thread_sptr_t                                           udp_thread_;

            volatile std::atomic_bool                               shut_down_;
            volatile std::atomic_bool                               started_;
            std::mutex                                              mutex_;

            unsigned int                                            port_;

        };

    }

}

#endif