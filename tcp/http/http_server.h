#pragma once

#include "../tcp_server.h"

namespace BrinK
{
    namespace tcp
    {
        class http_server final : public server
        {
        public:
            http_server();
            ~http_server();

        public:
            void start(const unsigned int& port = 80);

        protected:
            void handle_read(const boost::any&      client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const buff_sptr_t&                  buff);

            void handle_write(const boost::any&     client,
                const boost::system::error_code&    error,
                const size_t&                       bytes_transferred,
                const buff_sptr_t&                  buff);

        private:
            void read_handler(const tcp_client_sptr_t& c,
                const buff_sptr_t&                     b,
                const boost::system::error_code&       e,
                const size_t&                          s);

            void accept_handler(const tcp_client_sptr_t& c,
                const buff_sptr_t&                       b,
                const boost::system::error_code&         e,
                const size_t&                            s);

        private:
            std::string build_http_agent_(const std::string& file);

            size_t      get_http_header_length_(const buff_sptr_t& buff);

            bool        parse_http_header_(const buff_sptr_t&       buff,
                const param_uptr_t&                                 p,
                std::vector < std::string >&                        user_agent,
                std::vector < std::string >&                        proto);

            bool        do_http_request_(const tcp_client_sptr_t&   client,
                const std::string&                                  proto,
                const std::string&                                  file,
                const std::vector < std::string>&                   user_agent);
        };

    }

}
