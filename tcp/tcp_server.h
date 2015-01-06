#pragma once

#include <boost/noncopyable.hpp>
#include <atomic>

#include <brink_defines.h>
#include <pool/pool.hpp>
#include <pool/thread.hpp>
#include <brink_utils.h>

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
            server();
            virtual ~server();

        public:
            void start(const unsigned int& port,
                const complete_handler_t& recv_complete = nullptr,
                const complete_handler_t& send_complete = nullptr,
                const complete_handler_t& accept_complete = nullptr,
                const complete_handler_t& error_handler = nullptr,
                const complete_handler_t& timeout_handler = nullptr);

            void stop();

        public:
            void broadcast(const std::string& msg);

            void async_read(tcp_client_sptr_t client,
                const unsigned int& expect_size,
                const unsigned __int64& timeout_millseconds);

            void async_write(tcp_client_sptr_t client,
                const std::string& data);

        public:
            unsigned int get_port() const;

        private:
            void start_();
            void stop_();

        private:
            void accept_clients_();

        private:
            inline boost::asio::io_service&    get_io_service();

        private:
            void handle_accept(tcp_client_sptr_t client,
                const boost::system::error_code& error);

            void handle_timeout(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& timeout_count,
                const std::string& buff);

            void handle_error(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            void handle_read(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            void handle_write(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

        private:
            complete_handler_t                                          recv_handler_;
            complete_handler_t                                          send_handler_;
            complete_handler_t                                          error_handler_;
            complete_handler_t                                          accept_handler_;
            complete_handler_t                                          timeout_handler_;

            pool_sptr_t                                                 clients_pool_;

            std::vector<io_service_sptr_t>                              io_services_;
            std::atomic_size_t                                          io_service_pos_;
            std::list<work_sptr_t>                                      io_service_works_;
            std::list<thread_sptr_t>                                    io_service_threads_;

            io_service_sptr_t                                           acceptor_io_service_;
            tcp_acceptor_sptr_t                                         acceptor_;
            work_sptr_t                                                 acceptor_work_;
            thread_sptr_t                                               acceptor_thread_;

            std::atomic_bool                                            stopped_;
            std::atomic_bool                                            started_;

            std::mutex                                                  stop_mutex_;
            std::mutex                                                  accept_clients_mutex_;

            unsigned int                                                port_;
        };

    }

}
