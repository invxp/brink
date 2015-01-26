#include "http_server.h"

BrinK::tcp::http_server::http_server()
{

}

BrinK::tcp::http_server::~http_server()
{

}

void BrinK::tcp::http_server::start(const unsigned int& port)
{
    __super::start(port,
        std::bind(&tcp::http_server::accept_handler,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4),
        std::bind(&tcp::http_server::read_handler,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4));
}

void BrinK::tcp::http_server::handle_read(const boost::any& client,
    const boost::system::error_code&                        error,
    const size_t&                                           bytes_transferred,
    const char_sptr_t&                                      buff)
{
    __super::handle_read(client, error, bytes_transferred, buff);
}

void BrinK::tcp::http_server::handle_write(const boost::any& client,
    const boost::system::error_code&                         error,
    const size_t&                                            bytes_transferred,
    const char_sptr_t&                                       buff)
{
    __super::handle_write(client, error, bytes_transferred, buff);
}

void BrinK::tcp::http_server::read_handler(const tcp_client_sptr_t& c,
    const char_sptr_t&                                              b,
    const boost::system::error_code&                                e,
    const size_t&                                                   s)
{
    if (e || s == 0)
        return;

    c->get_param([this, &c, &b](const param_uptr_t& p)
    {
        if (!p->head_received)
        {
            std::vector < std::string > ua, proto;
            if (!parse_http_header_(b, p, ua, proto) || !do_http_request_(c, proto[0], proto[1], ua))
            {
                free_client(c);
                return;
            }

            // TODO
            b->get(get_http_header_length_(b), b->size(), [&p](char* b){ p->cache = b; });

        }
        else
        {

        }
        async_read(c, b, b->size());
    });
}

void BrinK::tcp::http_server::accept_handler(const tcp_client_sptr_t& c,
    const char_sptr_t&                                                b,
    const boost::system::error_code&                                  e,
    const size_t&                                                     s)
{
    if (e)
        return;

    char_sptr_t buffer = std::make_shared < BrinK::buffer >(4096);

    async_read(c, buffer, buffer->size(), 5000, [this](const char_sptr_t& buff)
    {
        return (this->get_http_header_length_(buff) == std::string::npos) ? false : true;
    });
}

size_t BrinK::tcp::http_server::get_http_header_length_(const char_sptr_t& buff)
{
    return BrinK::utils::c_find(buff->data(), "\r\n\r\n");
}

bool BrinK::tcp::http_server::parse_http_header_(const char_sptr_t& buff, const param_uptr_t& p, std::vector < std::string >& user_agent, std::vector < std::string >& proto)
{
    buff->get(get_http_header_length_(buff), [&user_agent, &proto](char* ch)
    {
        BrinK::utils::s_split(ch, "\r\n", user_agent);
        if (!user_agent.size())
            return;

        BrinK::utils::s_split(user_agent[0], " ", proto);
    });

    if (proto.size() < 2)
        return false;

    return true;
}

bool BrinK::tcp::http_server::do_http_request_(const tcp_client_sptr_t& client, const std::string& proto, const std::string& file, const std::vector < std::string>& user_agent)
{
    // TODO
    if (proto == "GET")
    {
        std::string buf;
        if (!BrinK::utils::file_to_string(file == "/" ? "index.html" : file, buf))
        { 
            async_write(client, "HTTP/1.1 404 NOT FOUND\r\n\r\n");
            client->free();
            return true;
        }

        std::string&& request = build_http_agent_(buf);

        async_write(client, request);
    }
    else if (proto == "POST")
    {
        int b = 0;
    }
    else
    {

    }

    return true;
}

std::string BrinK::tcp::http_server::build_http_agent_(const std::string& file)
{
    // TODO
    // This is only test
    std::string request = "HTTP/1.1 200 OK\r\n";
    request.append("Content-Type: text/html\r\n");
    request.append("Content-length: ");
    request.append(BrinK::utils::to_string < size_t >(file.length()));
    request.append("\r\n");
    request.append("\r\n");
    request.append(file);
    request.append("\r\n");
    request.append("\r\n");

    return request;
}

