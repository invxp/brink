#pragma once

#include <future>
#include <atomic>
#include <list>

namespace BrinK
{
    namespace pool
    {
        typedef std::function< void() >                 task_t;

        class async final
        {
        public:
            async()
            {
                stop_ = true;
                started_ = false;
            }
            ~async()
            {
                stop();
            }

        public:
            void post(const task_t& f)
            {
                std::unique_lock< std::mutex > lock(tasks_mutex_);

                tasks_.emplace_back(std::move(std::async([f, this]
                {
                    f();

                    std::unique_lock< std::mutex > lock(tasks_mutex_);
                    auto it = tasks_.begin();
                    while (it != tasks_.end())
                    {
                        if (it->wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready)
                            it = tasks_.erase(it);
                        else
                            ++it;
                    }
                })));
            }

            void dispatch(const task_t& f)
            {
                f();
            }

            bool wait()
            {
                if (stopped_())
                    return false;

                std::unique_lock< std::mutex > lock(tasks_mutex_);

                if (tasks_.empty())
                    return false;

                std::for_each(tasks_.begin(), tasks_.end(), [](std::future< void > &fu)
                {
                    fu.get();
                });

                tasks_.clear();

                return true;
            }

            bool start()
            {
                {
                    std::lock_guard< std::mutex > lock(mutex_);

                    if (started_)
                        return false;

                    started_ = true;

                    stop_ = false;

                }

                return true;
            }

            bool stop()
            {
                std::lock_guard< std::mutex > lock(mutex_);

                if (!started_)
                    return false;

                stop_ = true;

                started_ = false;

                return true;
            }

        private:
            bool stopped_()
            {
                std::lock_guard< std::mutex > lock(mutex_);

                return stop_ ? true : false;
            }

        private:
            std::list< std::future< void > >                        tasks_;
            std::mutex                                              tasks_mutex_;

            std::mutex                                              mutex_;
            std::atomic_bool                                        stop_;
            std::atomic_bool                                        started_;
        };

    }
}
