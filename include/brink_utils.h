#ifndef BRINK_UTILS_H
#define BRINK_UTILS_H

#include <brink_defines.h>
#include <sstream>

namespace BrinK
{
    namespace utils
    {
        inline std::string streambuf_to_string(streambuf_sptr_t sb)
        {
            std::istream is(sb.get());
            is.unsetf(std::ios_base::skipws);
            std::string s;
            s.append(std::istream_iterator < char > (is), std::istream_iterator < char > ());
            return s;
        }

        template <class T>
        inline std::string to_string(const T& v)
        {
            std::stringstream s;
            s << v;
            return s.str();
        }

        template <class T>
        inline T from_string(const std::string& s)
        {
            T t;
            std::stringstream ss(s);
            ss >> t;
            return t;
        }
    }

}

#endif