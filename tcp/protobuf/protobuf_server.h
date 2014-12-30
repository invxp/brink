#ifndef BRINK_TCP_PROTOBUF_SERVER_H
#define BRINK_TCP_PROTOBUF_SERVER_H

#include "../tcp_server.h"

#include <proto/pbuf.h>

namespace BrinK
{
    namespace tcp
    {
        class protobuf_server :public server
        {
        public:
            protobuf_server(const unsigned int& default_receive_len = 10,
                const unsigned __int64& default_socket_recv_timeout_millseconds = 30000);

            virtual ~protobuf_server();

        public:
            virtual void start(const unsigned int& port,
                const complete_handler_t& recv_complete = nullptr,
                const complete_handler_t& send_complete = nullptr,
                const complete_handler_t& accept_complete = nullptr,
                const complete_handler_t& error_handler = nullptr,
                const complete_handler_t& timeout_handler = nullptr);

        protected:
            virtual void handle_read(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            virtual void handle_write(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& bytes_transferred,
                const std::string& buff);

            virtual void handle_timeout(const boost::any& client,
                const boost::system::error_code& error,
                const size_t& timeout_count,
                const std::string& buff);

        };

    }

}
#endif