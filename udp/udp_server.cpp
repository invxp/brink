#include "udp_server.h"

BrinK::udp::server::server() :port_(0)
{
    started_ = false;
    shut_down_ = false;
}

BrinK::udp::server::~server()
{

}

bool BrinK::udp::server::start(const unsigned int& port)
{
    std::unique_lock < std::mutex > lock(mutex_);

    if (started_)
        return false;

    port_ = port;

    shut_down_ = false;

    start_();

    started_ = true;

    return started_;
}

void BrinK::udp::server::start_()
{
    udp_io_service_ = std::make_shared < boost::asio::io_service > ();

    udp_work_ = std::make_shared < boost::asio::io_service::work > (*udp_io_service_);

    udp_thread_ = std::make_shared < std::thread > ([this]{ boost::system::error_code ec; this->udp_io_service_->run(ec); });

    udp_host_ = std::make_shared< udp::socket >();

    udp_host_->reset(*udp_io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port_));

    udp_host_->async_receive();
}

void BrinK::udp::server::stop()
{
    std::unique_lock < std::mutex > lock(mutex_);

    if (!started_)
        return;

    shut_down_ = true;

    stop_();

    started_ = false;
}


void BrinK::udp::server::stop_()
{
    udp_work_.reset();
    udp_host_->closesocket();
    udp_thread_->join();
    udp_io_service_->reset();
    udp_host_.reset();
    udp_io_service_.reset();
    udp_thread_.reset();
}

unsigned int BrinK::udp::server::get_port()
{
    return port_;
}
