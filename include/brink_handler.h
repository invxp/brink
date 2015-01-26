#pragma once

#include <brink_buffer.h>
#include <boost/any.hpp>

typedef std::function < void(const boost::any&  any,
    const boost::system::error_code&            ec,
    const size_t&                               bytes_transferred,
    const char_sptr_t&                          buff) >             client_handler_t;
