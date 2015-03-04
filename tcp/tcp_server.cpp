#include "tcp_server.h"
#include <boost/bind.hpp>

BrinK::tcp::server::server() :
io_service_pos_(0),
port_(0),
acceptor_io_service_(nullptr),
acceptor_(nullptr),
acceptor_work_(nullptr),
acceptor_thread_(nullptr),
recv_handler_([this](const tcp_client_sptr_t& c, const buff_sptr_t& b, const boost::system::error_code& e, const size_t& s)
{
    // 接收到数据：这里可以发送数据到客户端或处理各种逻辑，s为接收的数据大小，b为元数据，c是该client，e为错误码
    // 默认先返回数据，在读N个字节，并设置超时（毫秒）
    if (e || !s)
        return;

    c->get_param([this, &c, &b, &s](const param_uptr_t& p)
    {
        async_write(c, b);
    });
}),
accept_handler_([this](const tcp_client_sptr_t& c, const buff_sptr_t& b, const boost::system::error_code& e, const size_t& s)
{
    // 握手：client初始化，通过c->get_param可以得到他的unique_id，规则："ip地址:端口"
    // 默认读N个字节并设置超时时间（毫秒）
    if (e)
        return;

    buff_sptr_t buff = std::make_shared< BrinK::buffer >(1024);
    async_read(c, buff, buff->size(), 3000);
}),
send_handler_([this](const tcp_client_sptr_t& c, const buff_sptr_t& b, const boost::system::error_code& e, const size_t& s)
{
    // 发送完成，b为发送完成的消息
    if (e || !s)
        return;

    c->get_param([this, &c, &b, &s](const param_uptr_t& p)
    {
        b->clear();
        async_read(c, b, b->size(), 1000);
    });
}
)
{
    stopped_ = true;

    started_ = false;

    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        io_services_.emplace_back(std::make_unique< boost::asio::io_service >());

    clients_pool_ = std::make_unique< pool::pool< tcp_client_sptr_t > >([this]
    {
        return tcp_client_sptr_t(std::make_shared< tcp::socket >(get_io_service_()));
    });

    thread_pool_ = std::make_unique< pool::async >();

    thread_pool_->start();
}

BrinK::tcp::server::~server()
{

}

void BrinK::tcp::server::start(const unsigned int& port,
    const complete_handler_t&                      accept_handler,
    const complete_handler_t&                      recv_handler,
    const complete_handler_t&                      send_handler)
{
    std::lock_guard< std::mutex > lock(stop_mutex_);

    if (started_)
        return;

    if (accept_handler)
        accept_handler_ = accept_handler;

    if (recv_handler)
        recv_handler_ = recv_handler;

    if (send_handler)
        send_handler_ = send_handler;

    port_ = port;

    stopped_ = false;

    started_ = true;

    start_();
}

void BrinK::tcp::server::start_()
{
    acceptor_io_service_ = std::make_unique< boost::asio::io_service >();

    acceptor_work_ = std::make_unique< boost::asio::io_service::work >(*acceptor_io_service_);

    acceptor_ = std::make_unique< boost::asio::ip::tcp::acceptor >(*acceptor_io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));

    acceptor_thread_ = std::make_unique< std::thread >([this]{ boost::system::error_code ec; acceptor_io_service_->run(ec); });

    std::for_each(io_services_.begin(), io_services_.end(), [this](const io_service_uptr_t& io)
    {
        io_service_works_.emplace_back(std::make_unique< boost::asio::io_service::work >(*io));
        io_service_threads_.emplace_back(std::make_unique< std::thread >([&io]{ boost::system::error_code ec; io->run(ec); }));
    });

    accept_clients();
}

void BrinK::tcp::server::stop()
{
    {
        std::lock_guard< std::mutex > lock_accept(accept_clients_mutex_);

        stopped_ = true;
    }

    std::lock_guard< std::mutex > lock_stop(stop_mutex_);

    if (!started_)
        return;

    stop_();

    started_ = false;
}

void BrinK::tcp::server::stop_()
{
    boost::system::error_code ec;

    // 取消监听端口，与io_service解开绑定，这里，未握手的socket会进入一次995    
    acceptor_work_.reset();
    acceptor_->cancel(ec);
    acceptor_thread_->join();
    acceptor_io_service_->reset();
    acceptor_.reset();
    acceptor_io_service_.reset();
    acceptor_thread_.reset();

    // 关闭所有客户端，并且取消timer，分别都会通过recv收到error消息
    clients_pool_->each([this](const tcp_client_sptr_t& client)
    {
        client->free();
    });

    // 等待所有io完成
    io_service_works_.clear();
    std::for_each(io_service_threads_.begin(), io_service_threads_.end(), [](const thread_uptr_t& td){ td->join(); });
    std::for_each(io_services_.begin(), io_services_.end(), [](const io_service_uptr_t& io){ io->reset(); });
    io_service_threads_.clear();

    thread_pool_->wait();
}

void BrinK::tcp::server::broadcast(const std::string& msg)
{
    std::lock_guard< std::mutex > lock(stop_mutex_);

    if ((stopped_) || (!started_))
        return;

    clients_pool_->each([this, &msg](const tcp_client_sptr_t& client)
    {
        async_write(client, msg);
    });
}

void BrinK::tcp::server::async_read(const tcp_client_sptr_t& client,
    buff_sptr_t                                              buffer,
    const size_t&                                            expect_size,
    const unsigned __int64&                                  millseconds,
    const pred_t&                                            pred)
{
    client->async_read(std::bind(&server::handle_read,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        buffer,
        expect_size,
        millseconds,
        pred);
}

void BrinK::tcp::server::async_write(const tcp_client_sptr_t& client, const std::string& data)
{
    client->async_write(std::bind(&server::handle_write,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        data);
}

void BrinK::tcp::server::async_write(const tcp_client_sptr_t& client, buff_sptr_t buffer)
{
    client->async_write(std::bind(&server::handle_write,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        buffer);
}

unsigned int BrinK::tcp::server::get_port() const
{
    return port_;
}

void BrinK::tcp::server::accept_clients()
{
    std::lock_guard< std::mutex > lock(accept_clients_mutex_);

    if (stopped_)
        return;

    clients_pool_->get([this](const tcp_client_sptr_t& client)
    {
        acceptor_->async_accept(client->raw_socket(),
            boost::bind(&server::handle_accept,
            this,
            client,
            boost::asio::placeholders::error));
    });
}

void BrinK::tcp::server::handle_accept(const tcp_client_sptr_t& client, const boost::system::error_code& error)
{
    // socket未握手之前，需单独处理，之后所有错误都在recv，这里如果出错，说明发生了比较严重的异常
    if (error)
        free_client(client);
    else
        client->accept();

    accept_clients();

    thread_pool_->post([this, client, error]
    {
        accept_handler_(client, nullptr, error, 1);
    });
}

boost::asio::io_service& BrinK::tcp::server::get_io_service_()
{
    boost::asio::io_service& io_service = *io_services_[io_service_pos_];

    ++io_service_pos_;

    if (io_service_pos_ >= io_services_.size())
        io_service_pos_ = 0;

    return io_service;
}

void BrinK::tcp::server::handle_read(const boost::any& client,
    const boost::system::error_code&                   error,
    const size_t&                                      bytes_transferred,
    const buff_sptr_t&                                 buff)
{
    const tcp_client_sptr_t& c = boost::any_cast< const tcp_client_sptr_t& >(client);

    if (error || !bytes_transferred)
        free_client(c);

    thread_pool_->post([this, c, buff, error, bytes_transferred]
    {
        recv_handler_(c, buff, error, bytes_transferred);
    });
}

void BrinK::tcp::server::handle_write(const boost::any& client,
    const boost::system::error_code&                    error,
    const size_t&                                       bytes_transferred,
    const buff_sptr_t&                                  buff)
{
    const tcp_client_sptr_t& c = boost::any_cast<const tcp_client_sptr_t&>(client);

    if (error || !bytes_transferred)
        free_client(c);

    thread_pool_->post([this, c, buff, error, bytes_transferred]
    {
        send_handler_(c, buff, error, bytes_transferred);
    });
}

void BrinK::tcp::server::free_client(const tcp_client_sptr_t& client)
{
    clients_pool_->free(client, [](const tcp_client_sptr_t& c)
    {
        c->free();
    });
}
