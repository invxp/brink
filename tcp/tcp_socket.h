#pragma once

#include <brink_defines.h>
#include <brink_utils.h>
#include <list>
#include <atomic>
#include <mutex>

// 定义Client扩展参数
namespace BrinK
{
    class param
    {
    public:
        param(){ reset(); }
        virtual ~param(){}

    public:
        void reset()
        {
            unique_id.clear();
            data.clear();
            binary.clear();
            reserve = 0;
            length = 0;
            type = 0;
            pbuf_head_received = false;
            error_count = 0;
        }

    public:
        std::string                 unique_id;              // ...add your param here
        std::string                 data;
        std::string                 binary;
        int                         reserve;
        int                         length;
        int                         type;
        bool                        pbuf_head_received;
        unsigned int                error_count;
    };
}

typedef std::shared_ptr < BrinK::param > param_sptr_t;

namespace BrinK
{
    namespace tcp
    {
        class socket : public std::enable_shared_from_this < BrinK::tcp::socket >
        {
        public:
            socket(boost::asio::io_service& io);
            virtual ~socket();

        public:
            boost::asio::ip::tcp::socket& raw_socket();

        public:
            // 用户自定义的结构
            void get_param(const std::function < void(param_sptr_t p) >& handler); 

        public:
            void accept();
            void free();
            void reset();

        public:
            void async_read(const size_t& expect_size, const client_handler_t& recv_handler);

            void async_write(const std::string& buff, const client_handler_t& write_handler);

            void async_timeout(const unsigned __int64& milliseconds, const client_handler_t& timeout_handler);

        private:
            void close_();

        private:
            void handle_read(const boost::system::error_code& error,
                size_t bytes_transferred,
                const client_handler_t& handler,
                const size_t& expect_size,
                streambuf_sptr_t sbuff,
                tcp_socket_sptr_t socket);

            void handle_write(const boost::system::error_code& error,
                size_t bytes_transferred,
                const client_handler_t& handler,
                const size_t& expect_size,
                streambuf_sptr_t sbuff,
                tcp_socket_sptr_t socket,
                const std::string& buff);

            void handle_timeout(const boost::system::error_code& error,
                timer_sptr_t timer,
                const unsigned __int64& time_out_milliseconds,
                const client_handler_t& handler);

        private:
            tcp_socket_sptr_t                           socket_;
            timer_sptr_t                                timer_;
            strand_sptr_t                               strand_;
            streambuf_sptr_t                            recv_buff_;

            std::list< streambuf_sptr_t >               send_buff_list_;    // 发送需要一个队列
            std::mutex                                  send_buff_mutex_;

            std::atomic_size_t                          timeout_count_;

            std::atomic_bool                            avalible_;
            std::mutex                                  avalible_mutex_;

            param_sptr_t                                param_;
            std::mutex                                  param_mutex_;

        };

    }

}
