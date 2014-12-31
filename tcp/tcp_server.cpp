#include "tcp_server.h"
#include <boost/bind.hpp>

BrinK::tcp::server::server() :
io_service_pos_(0),
port_(0),
acceptor_io_service_(nullptr),
acceptor_(nullptr),
acceptor_work_(nullptr),
acceptor_thread_(nullptr),
// 五种默认的server行为，如果需要外部监听，则在start函数定义
recv_handler_([this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
{
    // 接收到数据：这里可以发送数据到客户端或处理各种逻辑，这里e不需要关心，s为接收的数据大小，m为元数据，c是该client
    // 默认先返回数据，在读4个字节，并设置超时（毫秒）
    async_write(c, m);
    async_read(c, 4, 3000);
}),
accept_handler_([this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
{
    // 握手：client初始化，通过c->get_param可以得到他的unique_id，规则："ip地址:端口"，e、s通常不需要关心
    // 默认读4个字节
    async_read(c, 4, 2000);
}),
timeout_handler_([this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
{
    // 超时，m不需要关心，e为出错码，一般为995，s为超时次数
    // 默认关闭socket
    c->free();
}),
send_handler_([this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
{
    // 发送完成，m为发送完成的消息，e不参考，s为消息大小
    // std::cout << "Sended : " << m << std::endl;
}),
error_handler_([this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
{
    // 出错，此时已经回收该client、不需要关闭client和释放等任何操作，e为错误原因，其他可不参考
    // std::cout << "Error : " << e << std::endl;
})
{
    shut_down_ = false;

    started_ = false;

    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        io_services_.emplace_back(std::make_shared < boost::asio::io_service >());

    clients_pool_ = std::make_shared< pool::pool < tcp_client_sptr_t > >([this]
    {
        return tcp_client_sptr_t(std::make_shared < tcp::socket >(this->get_io_service()));
    }
    );
}

BrinK::tcp::server::~server()
{

}

void BrinK::tcp::server::start(const unsigned int& port,
    const complete_handler_t& recv_complete,
    const complete_handler_t& send_complete,
    const complete_handler_t& accept_complete,
    const complete_handler_t& error_handler,
    const complete_handler_t& timeout_handler)
{
    std::unique_lock < std::mutex > lock(mutex_);

    if (started_)
        return;

    if (recv_complete)
        recv_handler_ = recv_complete;

    if (send_complete)
        send_handler_ = send_complete;

    if (accept_complete)
        accept_handler_ = accept_complete;

    if (error_handler)
        error_handler_ = error_handler;

    if (timeout_handler)
        timeout_handler_ = timeout_handler;

    port_ = port;

    shut_down_ = false;

    start_();

    started_ = true;
}

void BrinK::tcp::server::start_()
{
    acceptor_io_service_ = std::make_shared < boost::asio::io_service >();

    acceptor_work_ = std::make_shared < boost::asio::io_service::work >(*acceptor_io_service_);

    acceptor_ = std::make_shared < boost::asio::ip::tcp::acceptor >(*acceptor_io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));

    acceptor_thread_ = std::make_shared < std::thread >([this]{ boost::system::error_code ec; this->acceptor_io_service_->run(ec); });

    std::for_each(io_services_.begin(), io_services_.end(), [this](io_service_sptr_t io)
    {
        io_service_works_.emplace_back(std::make_shared < boost::asio::io_service::work >(*io));
        io_service_threads_.emplace_back(std::make_shared < std::thread >([io]{ boost::system::error_code ec; io->run(ec); }));
    });

    accept_clients_();
}

void BrinK::tcp::server::stop()
{
    std::unique_lock < std::mutex > lock(mutex_);

    if (!started_)
        return;

    shut_down_ = true;

    stop_();

    started_ = false;
}

void BrinK::tcp::server::stop_()
{
    boost::system::error_code ec;

    // 取消监听端口，与io_service解开绑定，这里，未握手的socket会进入一次995
    acceptor_work_.reset();
    acceptor_->close(ec);
    acceptor_thread_->join();
    acceptor_io_service_->reset();
    acceptor_.reset();
    acceptor_io_service_.reset();
    acceptor_thread_.reset();

    // 关闭所有客户端，并且取消timer，分别都会通过recv收到error消息
    clients_pool_->each([this](tcp_client_sptr_t& client)
    {
        client->free();
    });

    // 等待所有io完成
    io_service_works_.clear();
    std::for_each(io_service_threads_.begin(), io_service_threads_.end(), [](thread_sptr_t& td){ td->join(); });
    std::for_each(io_services_.begin(), io_services_.end(), [](io_service_sptr_t& io){ io->reset(); });
    io_service_threads_.clear();
}

void BrinK::tcp::server::broadcast(const std::string& msg)
{
    std::unique_lock < std::mutex > lock(mutex_);

    if ((shut_down_) || (!started_))
        return;

    clients_pool_->each([this, msg](tcp_client_sptr_t& client)
    {
        if (client->raw_socket().is_open())
            client->async_write(msg,
            std::bind(&server::handle_write,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4));
    }
    );
}

void BrinK::tcp::server::async_read(tcp_client_sptr_t client, const unsigned int& expect_size, const unsigned __int64& timeout_millseconds)
{
    client->async_read(expect_size,
        std::bind(&server::handle_read,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4
        )
        );

    client->async_timeout(timeout_millseconds,
        std::bind(&server::handle_timeout,
        this, std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4)
        );
}

void BrinK::tcp::server::async_write(tcp_client_sptr_t client, const std::string& data)
{
    client->async_write(data,
        std::bind(&server::handle_write,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4));
}

unsigned int BrinK::tcp::server::get_port() const
{
    return port_;
}

void BrinK::tcp::server::accept_clients_()
{
    if (shut_down_)
        return;

    tcp_client_sptr_t& client = clients_pool_->get([this](tcp_client_sptr_t& client)
    {
        client->reset();

        this->acceptor_->async_accept(client->raw_socket(),
            boost::bind(&server::handle_accept,
            this,
            client,
            boost::asio::placeholders::error));
    });
}

void BrinK::tcp::server::handle_accept(tcp_client_sptr_t client, const boost::system::error_code& error)
{
    // socket未握手之前，需单独处理，之后所有错误都在recv处理
    if (error)
        handle_error(client, error, 0, "");
    else
    {
        client->accept();
        accept_handler_(boost::any_cast <tcp_client_sptr_t> (client), "", error, 0);
    }

    accept_clients_();
}

boost::asio::io_service& BrinK::tcp::server::get_io_service()
{
    boost::asio::io_service& io_service = *io_services_[io_service_pos_];

    ++io_service_pos_;

    if (io_service_pos_ >= io_services_.size())
        io_service_pos_ = 0;

    return io_service;
}

void BrinK::tcp::server::handle_error(const boost::any& client,
    const boost::system::error_code& error,
    const size_t& bytes_transferred,
    const std::string& buff)
{
    clients_pool_->free(boost::any_cast <tcp_client_sptr_t> (client), [](tcp_client_sptr_t& c){c->free(); });
    error_handler_(boost::any_cast <tcp_client_sptr_t> (client), buff, error, 0);
}

void BrinK::tcp::server::handle_read(const boost::any& client,
    const boost::system::error_code& error,
    const size_t& bytes_transferred,
    const std::string& buff)
{
    // 任何的异常请求，都会到handle_read里处理，它不仅负责接收数据，而且负责出错调用
    if (error)
        handle_error(client, error, bytes_transferred, buff);
    else
        recv_handler_(boost::any_cast <tcp_client_sptr_t> (client), buff, error, bytes_transferred);
}

void BrinK::tcp::server::handle_timeout(const boost::any& client,
    const boost::system::error_code& error,
    const size_t& timeout_count,
    const std::string& buff)
{
    timeout_handler_(boost::any_cast <tcp_client_sptr_t> (client), buff, error, timeout_count);
}

void BrinK::tcp::server::handle_write(const boost::any& client,
    const boost::system::error_code& error,
    const size_t& bytes_transferred,
    const std::string& buff)
{
    // send无论出错与否，不处理相关逻辑，否则会异常，防止资源竞争，统一交给recv处理
    send_handler_(boost::any_cast <tcp_client_sptr_t> (client), buff, error, bytes_transferred);
}
