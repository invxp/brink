#pragma once

#include <string>

namespace BrinK
{
    class param
    {
    public:
        param(){ reset(); }
        virtual ~param(){}

    public:
        void reset()
        {
            unique_id.clear();
            data.clear();
            binary.clear();
            header.clear();
            cache.clear();
            reserve = 0;
            length = 0;
            type = 0;
            head_received = false;
        }

    public:
        std::string                 unique_id;              // ...add your param here
        std::string                 data;
        std::string                 binary;
        std::string                 header;
        std::string                 cache;
        int                         reserve;
        int                         length;
        int                         type;
        bool                        head_received;
    };
}

typedef std::unique_ptr < BrinK::param >                                param_uptr_t;
