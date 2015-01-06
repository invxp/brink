#pragma once

#include <brink_defines.h>
#include <sstream>
#include <random>

namespace BrinK
{
    namespace utils
    {
        inline std::string streambuf_to_string(streambuf_sptr_t sb)
        {
            std::istream is(sb.get());
            is.unsetf(std::ios_base::skipws);
            return std::string(std::istream_iterator < char >(is), std::istream_iterator < char >());
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

        inline void sleep(const unsigned __int64& milliseconds)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        inline unsigned int random(const unsigned int& left, const unsigned int& right)
        {
            std::mt19937 rng((unsigned int)std::time(nullptr));
            std::uniform_int_distribution<unsigned int> num(left, right);
            return num(rng);
        }
    }

}
