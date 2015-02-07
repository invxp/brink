#include "tcp_socket.h"
#include <boost/bind.hpp>

BrinK::tcp::socket::socket(boost::asio::io_service& io) :
socket_(std::make_unique < boost::asio::ip::tcp::socket >(io)),
timer_(std::make_unique < boost::asio::deadline_timer >(io)),
strand_(std::make_unique < boost::asio::strand >(io)),
param_(std::make_unique < BrinK::param >())
{

}

BrinK::tcp::socket::~socket()
{

}

void BrinK::tcp::socket::close_()
{
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, ec);
    socket_->close(ec);
}

void BrinK::tcp::socket::cancel_timer_()
{
    boost::system::error_code ec;
    timer_->cancel(ec);
}

boost::asio::ip::tcp::socket& BrinK::tcp::socket::raw_socket()
{
    return *socket_;
}

void BrinK::tcp::socket::get_param(const std::function < void(const param_uptr_t& p) >& handler)
{
    std::lock_guard < std::mutex > lock_param(param_mutex_);

    handler(param_);
}

void BrinK::tcp::socket::accept()
{
    std::lock_guard < std::mutex > lock_param(param_mutex_);

    boost::system::error_code ec;
    param_->reset();
    param_->unique_id = socket_->remote_endpoint(ec).address().to_string(ec) +
        ":" +
        BrinK::utils::to_string < unsigned short >(socket_->remote_endpoint(ec).port());
}

void BrinK::tcp::socket::free()
{
    std::lock_guard < std::mutex > lock(mutex_);

    if (!socket_->is_open())
        return;

    cancel_timer_();
    close_();
}

void BrinK::tcp::socket::async_read(const client_handler_t& recv_handler,
    buff_sptr_t                                             buffer,
    const size_t&                                           expect_size,
    const unsigned __int64&                                 milliseconds,
    const pred_t&                                           predicate)
{
    std::lock_guard < std::mutex > lock(mutex_);

    if (!socket_->is_open())
        return;
    
    size_t expect_read = (expect_size > buffer->size()) ? buffer->size() : expect_size;

    socket_->async_read_some(
        boost::asio::buffer(buffer->raw(), expect_read),
        strand_->wrap(boost::bind(&socket::handle_read,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred,
        recv_handler,
        buffer,
        expect_read,
        predicate)));

    if (milliseconds > 0)
    {
        timer_->expires_from_now(boost::posix_time::milliseconds(milliseconds));

        timer_->async_wait(
            boost::bind(&socket::handle_timeout,
            shared_from_this(),
            boost::asio::placeholders::error,
            milliseconds
            ));
    }
}

void BrinK::tcp::socket::handle_read(const boost::system::error_code& error,
    const size_t&                                                     bytes_transferred,
    const client_handler_t&                                           handler,
    const buff_sptr_t&                                                buffer,
    const size_t&                                                     expect_size,
    const pred_t&                                                     pred)
{
    buffer->commit(bytes_transferred);

    if (error || !bytes_transferred || buffer->transferred() >= expect_size || pred(buffer))
    {
        cancel_timer_();
        handler(shared_from_this(), error, (bytes_transferred ? buffer->transferred() : bytes_transferred), buffer);
    }
    else
    {
        socket_->async_read_some(
            boost::asio::buffer(buffer->memory(), expect_size - buffer->transferred()),
            strand_->wrap(boost::bind(&socket::handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            buffer,
            expect_size,
            pred)));
    }
}

void BrinK::tcp::socket::async_write(const client_handler_t& write_handler, const std::string& data)
{
    std::lock_guard < std::mutex > lock(mutex_);

    if (!socket_->is_open())
        return;

    buff_sptr_t buffer = std::make_shared < BrinK::buffer >(data);

    socket_->async_write_some(
        boost::asio::buffer(buffer->raw(), buffer->size()),
        strand_->wrap(boost::bind(&tcp::socket::handle_write,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred,
        write_handler,
        buffer)));
}

void BrinK::tcp::socket::handle_write(const boost::system::error_code& error,
    const size_t&                                                      bytes_transferred,
    const client_handler_t&                                            handler,
    const buff_sptr_t&                                                 buffer
    )
{
    buffer->commit(bytes_transferred);

    if (error || !bytes_transferred || buffer->transferred() >= buffer->size())
        handler(shared_from_this(), error, (bytes_transferred ? buffer->transferred() : bytes_transferred), buffer);
    else
    {
        socket_->async_write_some(
            boost::asio::buffer(buffer->memory(), buffer->available()),
            strand_->wrap(boost::bind(&tcp::socket::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            buffer)));
    }
}

void BrinK::tcp::socket::handle_timeout(const boost::system::error_code& error, const unsigned __int64& milliseconds)
{
    if ((error) || (!socket_->is_open()))
        return;

    if (timer_->expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        cancel_timer_();
        close_();
    }
    else if (milliseconds > 0)
    {
        timer_->expires_from_now(boost::posix_time::milliseconds(milliseconds));

        timer_->async_wait(
            boost::bind(&socket::handle_timeout,
            shared_from_this(),
            boost::asio::placeholders::error,
            milliseconds
            ));
    }
}
