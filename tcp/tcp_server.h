#ifndef BRINK_TCP_SERVER_H
#define BRINK_TCP_SERVER_H

#include <boost/noncopyable.hpp>
#include <atomic>

#include <brink_defines.h>
#include <pool/pool.hpp>
#include <pool/thread.hpp>
#include <brink_utils.h>
#include <iostream>

#include "tcp_socket.h"

namespace BrinK
{

    namespace tcp
    {
        typedef std::shared_ptr < BrinK::tcp::socket >                                    tcp_client_sptr_t;
        typedef std::shared_ptr < BrinK::pool::pool< tcp_client_sptr_t > >                pool_sptr_t;
        typedef BrinK::pool::thread                                                       thread_pool_t;
        typedef std::function < void(tcp_client_sptr_t,
            const std::string&,
            const boost::system::error_code&,
            const size_t&) >
            complete_handler_t;

        class server :public boost::noncopyable
        {
        public:
            server(const unsigned int& default_receive_len = 15,
                const unsigned __int64& default_socket_recv_timeout_millseconds = 30000);
            virtual ~server();

        public:
            virtual void start(const unsigned int& port,
                const complete_handler_t& recv_complete = nullptr,
                const complete_handler_t& send_complete = nullptr,
                const complete_handler_t& accept_complete = nullptr,
                const complete_handler_t& error_handler = nullptr,
                const complete_handler_t& timeout_handler = nullptr);

            virtual void stop();

        public:
            void broadcast(const std::string& msg);
            void set_receive_length(const unsigned int& length);
            void set_timeout(const unsigned __int64& milliseconds);

        public:
            unsigned int get_port();

        protected:
            virtual void start_();
            virtual void stop_();

        protected:
            virtual void                        accept_clients();

        protected:
            virtual boost::asio::io_service&    get_io_service();

        protected:
            virtual void handle_accept(tcp_client_sptr_t client, const boost::system::error_code& error);

            virtual void handle_timeout(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& timeout_count,
                const std::string& buff);

            virtual void handle_error(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            virtual void handle_read(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            virtual void handle_write(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

        
        protected:
            unsigned int                                                default_recv_len_;
            unsigned __int64                                            default_socket_recv_timeout_millseconds_;

        private:
            complete_handler_t                                          recv_handler_;
            complete_handler_t                                          send_handler_;
            complete_handler_t                                          error_handler_;
            complete_handler_t                                          accept_handler_;
            complete_handler_t                                          timeout_handler_;

            thread_pool_t                                               thread_pool_;
            pool_sptr_t                                                 clients_pool_;

        private:
            std::vector<io_service_sptr_t>                              io_services_;
            std::mutex                                                  io_services_mutex_;

            std::list<work_sptr_t>                                      works_;
            std::list<thread_sptr_t>                                    threads_;

            io_service_sptr_t                                           acceptor_io_service_;
            tcp_acceptor_sptr_t                                         acceptor_;
            std::mutex                                                  acceptor_mutex_;

            work_sptr_t                                                 acceptor_work_;
            thread_sptr_t                                               acceptor_thread_;

            std::atomic_size_t                                          io_service_pos_;

            volatile std::atomic_bool                                   shut_down_;
            std::mutex                                                  mutex_;

            volatile std::atomic_bool                                   started_;
            unsigned int                                                port_;
        };

    }

}
#endif