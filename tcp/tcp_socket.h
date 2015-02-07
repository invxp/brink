#pragma once

#include <brink_defines.h>
#include <brink_handler.h>
#include <brink_buffer.h>
#include <brink_utils.h>
#include <brink_param.h>

#include <mutex>

namespace BrinK
{
    namespace tcp
    {
        class socket final: public std::enable_shared_from_this < BrinK::tcp::socket >
        {
        public:
            socket(boost::asio::io_service& io);
            ~socket();

        public:
            boost::asio::ip::tcp::socket& raw_socket();

        public:
            void get_param(const std::function < void(const param_uptr_t& p) >& handler); 

        public:
            void accept();
            void free();

        public:
            void async_read(const client_handler_t& recv_handler,
                buff_sptr_t                         buffer,
                const size_t&                       expect_size,
                const unsigned __int64&             milliseconds,
                const pred_t&                       predicate);

            void async_write(const client_handler_t& write_handler,
                const std::string&                   data);

        private:
            void handle_read(const boost::system::error_code&   error,
                const size_t&                                   bytes_transferred,
                const client_handler_t&                         handler,
                const buff_sptr_t&                              buffer,
                const size_t&                                   expect_size,
                const pred_t&                                   pred);

            void handle_write(const boost::system::error_code&  error,
                const size_t&                                   bytes_transferred,
                const client_handler_t&                         handler,
                const buff_sptr_t&                              buffer);

            void handle_timeout(const boost::system::error_code& error,
                const unsigned __int64&                          milliseconds);

        private:
            void close_();
            void cancel_timer_();

        private:
            tcp_socket_uptr_t                           socket_;
            timer_uptr_t                                timer_;
            strand_uptr_t                               strand_;
            param_uptr_t                                param_;

            std::mutex                                  mutex_;
            std::mutex                                  param_mutex_;
        };

    }

}

