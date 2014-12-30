#include "udp_socket.h"
#include <boost/bind.hpp>
#include <iostream>

BrinK::udp::socket::socket() :socket_(nullptr), strand_(nullptr), recv_buff_(nullptr), sender_endpoint_(nullptr)
{

}

BrinK::udp::socket::~socket()
{

}

boost::asio::ip::udp::socket& BrinK::udp::socket::raw_socket()
{
    return *socket_;
}

void BrinK::udp::socket::async_receive()
{
    socket_->async_receive_from(recv_buff_->prepare(1024),
        *sender_endpoint_,
        strand_->wrap(
        boost::bind(&socket::handle_receive_from, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred)));
}

void BrinK::udp::socket::handle_receive_from(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        // TODO
    }
    else
        free();
}

void BrinK::udp::socket::async_sendto(const std::string& buff)
{
    std::unique_lock < std::mutex > lock(send_buff_mutex_);
    send_buff_list_.emplace_back(std::make_shared < boost::asio::streambuf > ());
    std::ostream oa(send_buff_list_.front().get());

    oa << buff;

    socket_->async_send_to(send_buff_list_.front()->data(),
        *sender_endpoint_,
        boost::bind(&socket::handle_sendto,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred
        ));

}

void BrinK::udp::socket::handle_sendto(const boost::system::error_code& error, size_t bytes_transferred)
{

}

void BrinK::udp::socket::reset(boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint& ep)
{
    socket_ = std::make_shared < boost::asio::ip::udp::socket > (io_service, ep);
    strand_ = std::make_shared < boost::asio::strand > (io_service);
    recv_buff_ = std::make_shared < boost::asio::streambuf > ();
    sender_endpoint_ = std::make_shared < boost::asio::ip::udp::endpoint > ();
}

boost::asio::ip::udp::endpoint& BrinK::udp::socket::endpoint()
{
    return *sender_endpoint_;
}

void BrinK::udp::socket::closesocket()
{
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, ec);
    socket_->close(ec);
}

void BrinK::udp::socket::free()
{
    std::unique_lock < std::mutex > lock(send_buff_mutex_);
    send_buff_list_.clear();
}












