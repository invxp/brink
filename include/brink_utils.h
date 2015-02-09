#pragma once

#include <brink_defines.h>
#include <fstream>
#include <sstream>
#include <random>

namespace BrinK
{
    namespace utils
    {
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
            static std::mt19937 rng((unsigned int)std::time(nullptr));
            std::uniform_int_distribution<unsigned int> num(left, right);
            return num(rng);
        }

        inline size_t c_find(const char* org, const char* cond)
        {
            char* find = (char*)strstr(org, cond);

            if (!find)
                return std::string::npos;

            return (find - org) + (strlen(cond));
        }

        inline size_t s_split(const std::string& str, const std::string& del, std::vector< std::string >& ret)
        {
            std::string::size_type pos_begin = str.find_first_not_of(del);
            std::string::size_type pos_last = 0;

            while (pos_begin != std::string::npos)
            {
                std::string tmp = "";

                pos_last = str.find(del, pos_begin);

                if (pos_last != std::string::npos)
                {
                    tmp = str.substr(pos_begin, pos_last - pos_begin);
                    pos_begin = pos_last + del.length();
                }
                else
                {
                    tmp = str.substr(pos_begin);
                    pos_begin = pos_last;
                }

                if (!tmp.empty())
                {
                    ret.push_back(tmp);
                    tmp.clear();
                }

            }
            return ret.size();
        }

        inline bool file_to_string(const std::string& file, std::string& str)
        {
            std::ifstream ifs(file, std::ios::binary);

            if (!ifs.is_open())
                return false;

            ifs.seekg(0, std::ios::end);

            size_t file_size = ifs.tellg();

            if (file_size == std::string::npos)
                return false;

            std::unique_ptr < char[] > read_buf = std::make_unique < char[] >(file_size + sizeof(char));

            ifs.seekg(0);

            ifs.read(read_buf.get(), file_size);

            ifs.close();

            str.assign(read_buf.get(), file_size);

            return true;
        }
    }

}
