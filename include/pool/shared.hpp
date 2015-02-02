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
#include <list>

namespace BrinK
{
    namespace pool
    {
        template < class T, typename... Arg >

        class shared final
        {
        public:
            shared(const unsigned __int64& size = 32, Arg&&... args) : deleter_([this](T* ptr)
            {
                std::lock_guard < std::mutex > lock(mutex_);

                if (destructor_)
                    delete ptr;
                else
                    free_list_.emplace(std::shared_ptr< T >(ptr, this->deleter_));

            })
            {
                destructor_ = false;

                std::lock_guard < std::mutex > lock(mutex_);

                for (auto i = 0; i < size; ++i)
                    make_shared_(std::forward< Arg >(args)...);
            }

            ~shared()
            {
                destructor_ = true;
            }

        public:
            void get(Arg&&... args, const std::function < void(std::shared_ptr< T >&) >& op)
            {
                std::lock_guard < std::mutex > lock(mutex_);

                if (free_list_.empty())
                    make_shared_(std::forward< Arg >(args)...);

                op(std::move(free_list_.front()));

                free_list_.pop_front();
            }

            void each(const std::function < void(std::shared_ptr< T >& t) >& op = [](T&){})
            {
                std::lock_guard < std::mutex > lock(mutex_);

                std::for_each(free_list_.begin(), free_list_.end(), [&op](std::shared_ptr< T >& t){ op(t); });
            }

        private:
            void make_shared_(Arg && ...args)
            {
                free_list_.emplace(new T(std::forward< Arg >(args)...), deleter_);
            }

        private:
            std::mutex                                  mutex_;
            std::set< std::shared_ptr< T > >            free_list_;
            std::function< void(T*) >                   deleter_;
            std::atomic_bool                            destructor_;

        };

    }

}
