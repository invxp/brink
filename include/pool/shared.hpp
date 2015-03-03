/*
Created By InvXp 2014-12
共享对象池，自动回收资源
使用方式
pool::shared<obj> p;
pool.get(...);
*/
#pragma once

#include <atomic>
#include <mutex>
#include <set>

namespace BrinK
{
    namespace pool
    {
        template< class T, typename... Arg >

        class shared final
        {
        public:
            shared(const unsigned __int64& size = 32, Arg&&... args) : deleter_([this](T* ptr)
            {
                std::lock_guard< std::mutex > lock(mutex_);

                if (destructor_)
                    delete ptr;
                else
                    free_list_.emplace(std::shared_ptr< T >(ptr, deleter_));
            })
            {
                destructor_ = false;

                std::lock_guard< std::mutex > lock(mutex_);

                for (auto i = 0; i < size; ++i)
                    make_shared_(std::forward< Arg >(args)...);
            }

            ~shared()
            {
                destructor_ = true;
            }

        public:
            void get(Arg&&... args, const std::function< void(std::shared_ptr< T >&) >& op)
            {
                std::shared_ptr< T > p = nullptr;

                std::lock_guard< std::mutex > lock(mutex_);

                if (free_list_.empty())
                    p.reset(new T(std::forward< Arg >(args)...), deleter_);
                else
                {
                    p = *std::move(free_list_.begin());
                    free_list_.erase(free_list_.begin());
                }
                op(p);
            }

            void each(const std::function< void(std::shared_ptr< T >& t) >& op = [](T&){})
            {
                std::lock_guard< std::mutex > lock(mutex_);

                std::for_each(free_list_.begin(), free_list_.end(), [&op](std::shared_ptr< T >& t){ op(t); });
            }

        private:
            std::shared_ptr< T > make_shared_(Arg && ...args)
            {
                return *free_list_.emplace(new T(std::forward< Arg >(args)...), deleter_).first;
            }

        private:
            std::mutex                                  mutex_;
            std::set< std::shared_ptr< T > >            free_list_;
            std::function< void(T*) >                   deleter_;
            std::atomic_bool                            destructor_;

        };

    }

}
