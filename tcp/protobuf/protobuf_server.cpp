#include "protobuf_server.h"

BrinK::tcp::protobuf_server::protobuf_server()
{

}

BrinK::tcp::protobuf_server::~protobuf_server()
{

}

void BrinK::tcp::protobuf_server::start(const unsigned int& port)
{
    __super::start(port,
        std::bind(&tcp::protobuf_server::accept_handler,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        std::bind(&tcp::protobuf_server::read_handler,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        std::bind(&tcp::protobuf_server::write_handler,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4)
        );
}

void BrinK::tcp::protobuf_server::handle_read(const boost::any& client,
    const boost::system::error_code&                            error,
    const size_t&                                               bytes_transferred,
    const buff_sptr_t&                                          buff)
{
    __super::handle_read(client, error, bytes_transferred, buff);
}

void BrinK::tcp::protobuf_server::handle_write(const boost::any& client,
    const boost::system::error_code&                             error,
    const size_t&                                                bytes_transferred,
    const buff_sptr_t&                                           buff)
{
    __super::handle_write(client, error, bytes_transferred, buff);
}

void BrinK::tcp::protobuf_server::read_handler(const tcp_client_sptr_t& c,
    const buff_sptr_t&                                                  b,
    const boost::system::error_code&                                    e,
    const size_t&                                                       s)
{
    if (e || !s)
        return;

    c->get_param([this, &c, &b, &s](const param_uptr_t& p)
    {
        if (!p->head_received)
        {
            pbuf_head head;
            if (!head.ParseFromArray(b->data(), s))
            {
                free_client(c);
                return;
            }
            p->head_received = true;
            p->length = head.body_length();
            p->type = head.type();

            b->clear();

            async_read(c, b, (b->alloc(p->length) ? p->length : 0));
        }
        else
        {
            pbuf_body body;
            if (!body.ParseFromArray(b->data(), s))
            {
                free_client(c);
                return;
            }
            p->head_received = false;
            p->binary = body.binary();
            p->data = body.data();
            p->reserve = body.reserve();

            b->clear();

            async_read(c, b, (b->alloc(PROTOBUF_HEAD_LENGTH) ? PROTOBUF_HEAD_LENGTH : 0));
        }
    });
}

void BrinK::tcp::protobuf_server::write_handler(const tcp_client_sptr_t& c, const buff_sptr_t& b, const boost::system::error_code& e, const size_t& s)
{
    if (e || !s)
        return;
}

void BrinK::tcp::protobuf_server::accept_handler(const tcp_client_sptr_t& c,
    const buff_sptr_t&                                                    b,
    const boost::system::error_code&                                      e,
    const size_t&                                                         s)
{
    buff_sptr_t buffer = std::make_shared< BrinK::buffer >(PROTOBUF_HEAD_LENGTH);

    async_read(c, buffer, buffer->size());
}

