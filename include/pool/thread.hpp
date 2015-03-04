#pragma once

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <algorithm>

namespace BrinK
{
    namespace pool
    {
        typedef std::function< void() >                 task_t;

        typedef std::unique_ptr< std::thread >          thread_uptr_t;

        class thread final
        {
        public:
            thread()
            {
                stop_ = true;
                started_ = false;
            }
            ~thread()
            {
                stop();
                clear();
            }

        public:
            void post(const task_t& f)
            {
                std::unique_lock< std::mutex > lock(tasks_mutex_);

                tasks_.emplace_back(f);
                awake_condition(lock, tasks_condition_);
            }

            void dispatch(const task_t& f)
            {
                f();
            }

            bool wait()
            {
               return wait_(wait_mutex_, wait_condition_);
            }

            bool start(const unsigned int& thread_count = std::thread::hardware_concurrency())
            {
                std::lock_guard< std::mutex > lock(mutex_);

                if (started_)
                    return false;

                started_ = true;

                stop_ = false;

                create_threads_(thread_count);

                return true;
            }

            bool stop()
            {
                std::lock_guard< std::mutex > lock(mutex_);

                {
                    std::unique_lock< std::mutex > lock_task(tasks_mutex_);

                    if (!started_)
                        return false;

                    stop_ = true;

                    awake_condition(lock_task, tasks_condition_);
                }

                remove_threads_();

                std::unique_lock< std::mutex > lock_all(wait_mutex_);
                awake_condition(lock_all, wait_condition_);

                started_ = false;

                return true;
            }

            void clear()
            {
                std::unique_lock< std::mutex > lock(tasks_mutex_);

                tasks_.clear();
            }

            size_t size()
            {
                std::unique_lock< std::mutex > lock(tasks_mutex_);

                return tasks_.size();
            }

        private:
            void pool_func_()
            {
                task_t task;
                while (get_task_(task))
                {
                    task();
                    std::this_thread::yield();
                }
            }

            bool get_task_(task_t& t)
            {
                std::unique_lock< std::mutex > lock_task(tasks_mutex_);

                tasks_condition_.wait(lock_task, [this]
                {
                    if (stop_)
                        return true;

                    std::unique_lock< std::mutex > lock_all(wait_mutex_);

                    if (tasks_.empty())
                    {
                        awake_condition(lock_all, wait_condition_);
                        return false;
                    }
                    return true;
                });

                if (stop_)
                    return false;

                t = std::move(tasks_.front());
                tasks_.pop_front();
                return true;
            }

            void create_threads_(const unsigned int & pool_size)
            {
                for (unsigned int i = 0; i < pool_size; i++)
                    threads_.emplace_back(std::make_unique< std::thread >(std::bind(&thread::pool_func_, this)));
            }

            void remove_threads_()
            {
                std::for_each(threads_.begin(), threads_.end(), [](const thread_uptr_t& td)
                { 
                    td->join();
                });
                threads_.clear();
            }

            bool stopped_()
            {
                std::lock_guard< std::mutex > lock(mutex_);

                return stop_ ? true : false;
            }

            void awake_condition(std::unique_lock< std::mutex >& mutex, std::condition_variable& cond)
            {
                cond.notify_all();
                mutex.unlock();
            }

            bool wait_(std::mutex& mutex, std::condition_variable& cond)
            {
                if (stopped_())
                    return false;

                std::unique_lock< std::mutex > lock_task(tasks_mutex_);

                if (tasks_.empty())
                    return false;

                std::unique_lock< std::mutex > lock_wait(mutex);

                lock_task.unlock();
                cond.wait(lock_wait);
                return true;
            }

        private:
            std::list< task_t >                                     tasks_;
            std::condition_variable                                 tasks_condition_;
            std::mutex                                              tasks_mutex_;

            std::list< thread_uptr_t >                              threads_;

            std::mutex                                              mutex_;
            std::atomic_bool                                        stop_;
            std::atomic_bool                                        started_;

            std::mutex                                              wait_mutex_;
            std::condition_variable                                 wait_condition_;
        };

    }
}
