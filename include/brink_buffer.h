/*
Created By InvXp 2014-12
数据缓冲区
使用方式
类似boost::asio::streambuf
造轮子原因：streambuf commit后无法还原得到之前的指针
*/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <atomic>

namespace BrinK
{
    class buffer final
    {
    public:
        buffer(const size_t& size = 0) : pos_(0), spliter_('\0')
        {
            alloc(size);
        }

        buffer(const std::string& data) : pos_(0), spliter_('\0')
        {
            *this = data;
        }

        ~buffer()
        {

        }

    public:
        bool alloc(const size_t& length)
        {
            if (length > max_size())
                return false;

            if (length < size())
                return true;

            buffer_.resize(length);
            return (size() > length) ? true : false;
        }

        void clear()
        {
            pos_ = 0;
            buffer_.clear();
        }

        size_t size() const
        {
            return buffer_.size();
        }

        char* memory()
        {
            return (transferred() == size()) ? nullptr : &buffer_[transferred()];
        }

        void commit(const size_t& length)
        {
            pos_ = ((length >= (size() - transferred())) ? size() : (transferred() + length));
        }

        const char* data() const
        {
            return buffer_.data();
        }

        size_t transferred() const
        {
            return pos_;
        }

        size_t max_size() const
        {
            return buffer_.max_size();
        }

        size_t available() const
        {
            return size() - transferred();
        }

        std::vector < char >& raw()
        {
            return buffer_;
        }

        void get(const size_t& count, const std::function< void(char*) >& op)
        {
            get(0, count, op);
        }

        void get(const size_t& off, const size_t& count, const std::function< void(char*) >& op)
        {
            if (off >= size())
                return;

            char* p = buffer_.data();

            p += off;

            size_t max_count = ((size() - off) < count) ? 0 : count;

            if (!max_count)
                op(p);
            else
            {
                std::swap(spliter_, p[max_count]);
                op(p);
                std::swap(spliter_, p[max_count]);
            }
        }

        void operator+=(const std::string& str)
        {
            std::copy(str.begin(), str.end(), std::back_inserter(buffer_));
        }

        void operator=(const std::string& str)
        {
            buffer_.assign(str.begin(), str.end());
        }

    private:
        std::vector < char >                     buffer_;
        std::atomic_size_t                       pos_;
        char                                     spliter_;

    };
}

typedef std::shared_ptr < BrinK::buffer >                               char_sptr_t;
typedef std::function < bool(const char_sptr_t&) >                      pred_t;
