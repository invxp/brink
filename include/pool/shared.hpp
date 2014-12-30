/*
Created By InvXp 2014-12
共享对象池，自动回收资源
使用方式
pool::shared<obj> p;
auto p = pool.get();
p->xxx
*/

#ifndef BRINK_POOL_SHARED_H
#define BRINK_POOL_SHARED_H

#include <mutex>
#include <list>
#include <atomic>

namespace BrinK
{
    namespace pool
    {
        template < class T, typename... Arg >

        class shared
        {
        public:
            shared(const unsigned __int64& size = 32, Arg&&... args) : deleter_([this](T* ptr)
            {
                destructor_ ? delete ptr : free_list_.emplace_back(std::shared_ptr< T >(ptr, this->deleter_));
            }
                )
            {
                destructor_ = false;

                std::unique_lock < std::mutex > lock(mutex_);

                for (auto i = 0; i < size; ++i)
                    make_shared_(std::forward< Arg >(args)...);
            }

            virtual ~shared()
            {
                std::unique_lock < std::mutex > lock(mutex_);

                destructor_ = true;
                free_list_.clear();
            }

        public:
            std::shared_ptr < T > get(Arg&&... args, const std::function < void(T&) >& op)
            {
                std::unique_lock < std::mutex > lock(mutex_);

                std::shared_ptr < T > sptr = get_(std::forward< Arg >(args)...);
                op(*sptr);
                return sptr;
            }

            std::shared_ptr < T > get(Arg&&... args)
            {
                std::unique_lock < std::mutex > lock(mutex_);

                return get_(std::forward< Arg >(args)...);
            }

            void free(std::shared_ptr < T >& t, const std::function < void(T&) >& op = [](T&){})
            {
                std::shared_ptr< T > ptr = std::move(t);
                op(*ptr);
                ptr.reset();
            }

        public:
            size_t size() const
            {
                std::unique_lock < std::mutex > lock(mutex_);

                return free_list_.size();
            }

        public:
            void each(const std::function < void(T& t) >& op = [](T&){})
            {
                std::unique_lock < std::mutex > lock(mutex_);

                std::for_each(free_list_.begin(), free_list_.end(), [&op](T& t){op(t); });
            }

        private:
            void make_shared_(Arg && ...args)
            {
                free_list_.emplace_back(new T(std::forward< Arg >(args)...), deleter_);
            }

            std::shared_ptr < T > get_(Arg&&... args)
            {
                if (free_list_.empty())
                    make_shared_(std::forward< Arg >(args)...);

                std::shared_ptr < T > sptr = std::move(free_list_.front());

                free_list_.pop_front();

                return sptr;
            }

        private:
            std::mutex                                  mutex_;
            std::list< std::shared_ptr< T > >           free_list_;
            std::function< void(T*) >                   deleter_;
            volatile std::atomic_bool                   destructor_;

        };

    }

}

#endif