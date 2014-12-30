#include "protobuf_server.h"

BrinK::tcp::protobuf_server::protobuf_server(const unsigned int& default_receive_len /*= 10*/,
    const unsigned __int64& default_socket_recv_timeout_millseconds /*= 30000*/) 
{
    set_receive_length(default_receive_len);
    set_timeout(default_socket_recv_timeout_millseconds);
}

BrinK::tcp::protobuf_server::~protobuf_server()
{

}

void BrinK::tcp::protobuf_server::start(const unsigned int& port, const complete_handler_t& recv_complete /*= nullptr*/, const complete_handler_t& send_complete /*= nullptr*/, const complete_handler_t& accept_complete /*= nullptr*/, const complete_handler_t& error_handler /*= nullptr*/, const complete_handler_t& timeout_handler /*= nullptr*/)
{
    auto recv_handler = [this](tcp_client_sptr_t c, const std::string& m, const boost::system::error_code& e, const size_t& s)
    {
        c->get_param([this,c,&m](param_sptr_t p)
        {
            if (!p->pbuf_head_received)
            {
                pbuf_head head;
                if (!head.ParseFromString(m))
                {
                    ++p->error_count;
                    return;
                }
                p->pbuf_head_received = true;
                p->length = head.length();
                p->type = head.type();

                c->async_read(p->length,
                    std::bind(&protobuf_server::handle_read,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4),
                    default_socket_recv_timeout_millseconds_,
                    std::bind(&protobuf_server::handle_timeout,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4));
            }
            else
            {
                pbuf_body body;
                if (!body.ParseFromString(m))
                {
                    ++p->error_count;
                    return;
                }
                p->pbuf_head_received = false;
                p->length = body.length();
                p->type = body.type();
                p->binary = body.binary();
                p->data = body.str();
                p->reserve = body.reserve();

                c->async_write("OK",
                    std::bind(&protobuf_server::handle_write,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4));

                c->async_read(default_recv_len_,
                    std::bind(&protobuf_server::handle_read,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4),
                    default_socket_recv_timeout_millseconds_,
                    std::bind(&protobuf_server::handle_timeout,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4));
            }
        });
    };

    __super::start(port, recv_handler, send_complete, accept_complete, error_handler, timeout_handler);
}

void BrinK::tcp::protobuf_server::handle_read(const boost::any& client, const boost::system::error_code& error, const size_t& bytes_transferred, const std::string& buff)
{
    __super::handle_read(client, error, bytes_transferred, buff);
}

void BrinK::tcp::protobuf_server::handle_write(const boost::any& client, const boost::system::error_code& error, const size_t& bytes_transferred, const std::string& buff)
{
    __super::handle_write(client, error, bytes_transferred, buff);
}

void BrinK::tcp::protobuf_server::handle_timeout(const boost::any& client, const boost::system::error_code& error, const size_t& timeout_count, const std::string& buff)
{
    __super::handle_timeout(client, error, timeout_count, buff);
}





