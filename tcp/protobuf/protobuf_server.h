#ifndef BRINK_TCP_PROTOBUF_SERVER_H
#define BRINK_TCP_PROTOBUF_SERVER_H

#include "../tcp_server.h"

#include "include/pbuf.h"

namespace BrinK
{
    namespace tcp
    {
        class protobuf_server final :public server
        {
        public:
            protobuf_server();
            ~protobuf_server();

        public:
            void start(const unsigned int& port);

        protected:
            void handle_read(const boost::any&      client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const char_sptr_t&                  buff);

            void handle_write(const boost::any&     client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const char_sptr_t&                  buff);

        private:
            void read_handler(const tcp_client_sptr_t&      c,
                const char_sptr_t&                          b,
                const boost::system::error_code&            e,
                const size_t&                               s);

            void accept_handler(const tcp_client_sptr_t&    c,
                const char_sptr_t&                          b,
                const boost::system::error_code&            e,
                const size_t&                               s);

        };

    }

}
#endif