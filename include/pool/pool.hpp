/*
Created By InvXp 2014-11
对象池
使用方式
初始化需要传递该对象的constructor，且分配一个大小（会递增）
pool<obj> p([]{return *new obj(...); }, 32);
auto o=p.get([](obj&){...});
p.free(o,[](obj&){...});
p.each...
*/

#pragma once

#include <mutex>
#include <list>

namespace BrinK
{
    namespace pool
    {
        template < class T >

        class pool
        {
        public:
            pool(const std::function < T() >& constructor, const unsigned __int64& size = 32) :new_(constructor)
            {
                std::unique_lock < std::mutex > lock(mutex_);
                for (auto i = 0; i < size; ++i)
                    free_list_.emplace_back(new_());
            }

            virtual ~pool()
            {
                std::unique_lock < std::mutex > lock(mutex_);
                busy_list_.clear();
                free_list_.clear();
            }
        public:
            T& get(const std::function < void(T& t) >& op = [](T&){})
            {
                std::unique_lock < std::mutex > lock(mutex_);

                if (free_list_.empty())
                    busy_list_.emplace_back(new_());
                else
                {
                    busy_list_.emplace_back(free_list_.front());
                    free_list_.pop_front();
                }

                op(busy_list_.back());

                return busy_list_.back();

            }
            void free(const T& t, const std::function < void(T& t) >& op = [](T&){})
            {
                std::unique_lock < std::mutex > lock(mutex_);
                free_list_.emplace_back(t);
                busy_list_.remove(free_list_.back());
                op(free_list_.back());
            }

        public:
            size_t busy_size()
            {
                std::unique_lock < std::mutex > lock(mutex_);
                return busy_list_.size();
            }
            size_t free_size()
            {
                std::unique_lock < std::mutex > lock(mutex_);
                return free_list_.size();
            }

        public:
            void each(const std::function < void(T& t) >& op = [](T&){})
            {
                std::unique_lock < std::mutex > lock(mutex_);
                std::for_each(busy_list_.begin(), busy_list_.end(), [&op](T& t){op(t); });
            }

            void clear(const std::function < void(T& t) >& op = [](T&){})
            {
                std::unique_lock < std::mutex > lock(mutex_);
                std::for_each(busy_list_.begin(), busy_list_.end(), [&op](T& t){op(t); });
                std::copy(busy_list_.begin(), busy_list_.end(), std::back_inserter(free_list_));
                busy_list_.clear();
            }

        private:
            std::mutex                                  mutex_;
            std::list< T >                              busy_list_;
            std::list< T >                              free_list_;
            std::function< T() >                        new_;

        };

    }

}
