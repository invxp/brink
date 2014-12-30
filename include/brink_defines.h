#ifndef BRINK_DEFINES_H
#define BRINK_DEFINES_H

#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include <boost/any.hpp>

typedef std::shared_ptr < boost::asio::io_service >                  io_service_sptr_t;
typedef std::shared_ptr < boost::asio::io_service::work >            work_sptr_t;

typedef std::shared_ptr < std::thread >                              thread_sptr_t;

typedef std::shared_ptr < boost::asio::ip::tcp::socket >             tcp_socket_sptr_t;
typedef std::shared_ptr < boost::asio::ip::udp::socket >             udp_socket_sptr_t;

typedef std::shared_ptr < boost::asio::ip::tcp::acceptor >           tcp_acceptor_sptr_t;
typedef std::shared_ptr < boost::asio::ip::udp::endpoint >           udp_endport_sptr_t;

typedef std::shared_ptr < boost::asio::deadline_timer >              timer_sptr_t;

typedef std::shared_ptr < boost::asio::strand >                      strand_sptr_t;

typedef std::shared_ptr < boost::asio::streambuf >                   streambuf_sptr_t;

typedef std::function < void() >                                     bind_handler_t;

typedef std::function < void(const boost::any& any,
    const boost::system::error_code& ec,
    const size_t& bytes_transferred,
    const std::string& buff) >
    client_handler_t;

#endif