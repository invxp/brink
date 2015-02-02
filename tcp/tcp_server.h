#pragma once

#include <boost/noncopyable.hpp>
#include <atomic>
#include <brink_defines.h>
#include <brink_utils.h>
#include <pool/pool.hpp>

#include "tcp_socket.h"

namespace BrinK
{
    namespace tcp
    {
        typedef std::shared_ptr < BrinK::tcp::socket >                                    tcp_client_sptr_t;

        typedef std::unique_ptr < BrinK::pool::pool< tcp_client_sptr_t > >                pool_uptr_t;

        typedef std::function < void(const tcp_client_sptr_t&,
            const buff_sptr_t&,
            const boost::system::error_code&,
            const size_t&) >                                                              complete_handler_t;

        class server :public boost::noncopyable
        {
        public:
            server();
            virtual ~server();

        public:
            void start(const unsigned int& port,
                const complete_handler_t&  accept_handler = nullptr,
                const complete_handler_t&  recv_handler = nullptr,
                const complete_handler_t&  send_handler = nullptr
                );

            void stop();

        public:
            void broadcast(const std::string& msg);

            void async_read(const tcp_client_sptr_t& client,
                buff_sptr_t                          buffer,
                const size_t&                        expect_size,
                const unsigned __int64&              milliseconds = 0,
                const pred_t&                        predicate = [](const buff_sptr_t&){ return false; });

            void async_write(const tcp_client_sptr_t& client,
                 const std::string&                   data);

        public:
            unsigned int get_port() const;

        protected:
            void accept_clients();
            void free_client(const tcp_client_sptr_t& client);

        protected:
            void handle_accept(const tcp_client_sptr_t& client,
                const boost::system::error_code&        error);

            void handle_read(const boost::any&      client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const buff_sptr_t&                  buff);

            void handle_write(const boost::any&     client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const buff_sptr_t&                  buff);

        private:
            void start_();
            void stop_();

        private:
            inline boost::asio::io_service&    get_io_service_();

        private:
            complete_handler_t                                          accept_handler_;
            complete_handler_t                                          recv_handler_;
            complete_handler_t                                          send_handler_;

            pool_uptr_t                                                 clients_pool_;

            std::vector < io_service_uptr_t >                           io_services_;
            std::atomic_size_t                                          io_service_pos_;
            std::list < work_uptr_t >                                   io_service_works_;
            std::list < thread_uptr_t >                                 io_service_threads_;

            io_service_uptr_t                                           acceptor_io_service_;
            tcp_acceptor_uptr_t                                         acceptor_;
            work_uptr_t                                                 acceptor_work_;
            thread_uptr_t                                               acceptor_thread_;

            std::atomic_bool                                            stopped_;
            std::atomic_bool                                            started_;

            std::mutex                                                  stop_mutex_;
            std::mutex                                                  accept_clients_mutex_;

            unsigned int                                                port_;
        };

    }

}
