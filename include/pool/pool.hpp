/*
Created By InvXp 2014-11
对象池
使用方式
初始化需要传递该对象的constructor，且分配一个大小（会递增）
pool<obj> p([]{return *new obj(...); }, 32);
p.get([](obj&){...});
p.free(o,[](obj&){...});
p.each...
*/

#pragma once

#include <mutex>
#include <set>

namespace BrinK
{
    namespace pool
    {
        template < class T >

        class pool final
        {
        public:
            pool(const std::function < T() >& constructor, const unsigned __int64& size = 32) :new_(constructor)
            {
                std::lock_guard < std::mutex > lock(mutex_);

                for (auto i = 0; i < size; ++i)
                    free_list_.emplace(new_());
            }

            ~pool()
            {
                std::lock_guard < std::mutex > lock(mutex_);

                busy_list_.clear();
                free_list_.clear();
            }

        public:
            void get(const std::function < void(const T& t) >& op = [](const T&){})
            {
                std::lock_guard < std::mutex > lock(mutex_);
                
                if (free_list_.empty()) 
                    op(*busy_list_.emplace(new_()).first);
                else
                {
                    auto p = busy_list_.emplace(*std::move(free_list_.begin()));
                    free_list_.erase(free_list_.begin());
                    op(*p.first);
                }
            }

            void free(const T& t, const std::function < void(const T& t) >& op = [](const T&){})
            {
                std::lock_guard < std::mutex > lock(mutex_);

                auto it = busy_list_.find(t);
                if (it == busy_list_.end())
                    return;

                free_list_.emplace(t);
                busy_list_.erase(it);
                op(t);
            }

            void each(const std::function < void(const T& t) >& op = [](const T&){})
            {
                std::lock_guard < std::mutex > lock(mutex_);

                std::for_each(busy_list_.begin(), busy_list_.end(), [&op](const T& t){ op(t); });
            }

        private:
            std::mutex                                  mutex_;
            std::set< T >                               busy_list_;
            std::set< T >                               free_list_;
            std::function< T() >                        new_;

        };

    }

}
