#ifndef BRINK_UDP_SOCKET_H
#define BRINK_UDP_SOCKET_H

#include <boost/array.hpp>
#include <brink_defines.h>
#include <mutex>

namespace BrinK
{
    namespace udp
    {
        class socket : public std::enable_shared_from_this < BrinK::udp::socket >
        {
        public:
            socket();
            virtual ~socket();

        public:
            boost::asio::ip::udp::socket&   raw_socket();
            boost::asio::ip::udp::endpoint& endpoint();

        public:
            virtual void async_receive();
            virtual void async_sendto(const std::string& buff);

        public:
            virtual void reset(boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint& ep);
            virtual void free();
            virtual void closesocket();

        protected:
            virtual void handle_receive_from(const boost::system::error_code& error, size_t bytes_transferred);
            virtual void handle_sendto(const boost::system::error_code& error, size_t bytes_transferred);

        private:
            udp_socket_sptr_t                       socket_;
            udp_endport_sptr_t                      sender_endpoint_;
            streambuf_sptr_t                        recv_buff_;
            strand_sptr_t                           strand_;

            std::list < streambuf_sptr_t >          send_buff_list_;
            std::mutex                              send_buff_mutex_;

        };

    }

}

#endif