#pragma once

#include <thread>
#include <memory>
#include <boost/asio.hpp>

typedef std::unique_ptr < boost::asio::io_service >                  io_service_uptr_t;
typedef std::unique_ptr < boost::asio::io_service::work >            work_uptr_t;

typedef std::unique_ptr < std::thread >                              thread_uptr_t;

typedef std::unique_ptr < boost::asio::ip::tcp::acceptor >           tcp_acceptor_uptr_t;
typedef std::unique_ptr < boost::asio::ip::udp::endpoint >           udp_endport_uptr_t;

typedef std::unique_ptr < boost::asio::strand >                      strand_uptr_t;

typedef std::unique_ptr < boost::asio::ip::tcp::socket >             tcp_socket_uptr_t;
typedef std::unique_ptr < boost::asio::ip::udp::socket >             udp_socket_uptr_t;

typedef std::unique_ptr < boost::asio::deadline_timer >              timer_uptr_t;

typedef std::unique_ptr < std::string >                              string_uptr_t;
