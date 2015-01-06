#ifndef BRINK_POOL_THREAD_H
#define BRINK_POOL_THREAD_H

#include <list>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <algorithm>
#include <atomic>

namespace BrinK
{
    namespace pool
    {
        typedef std::function < void() >                 task_t;

        typedef std::shared_ptr < std::thread >          thread_ptr_t;

        class thread
        {
        public:
            thread()
            {
                stop_ = false;
            }
            virtual ~thread()
            {
                stop();
                clear();
            }

        public:
            void post(const task_t& f)
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
                tasks_.emplace_back(f);
                awake_condition(lock, tasks_condition_);
            }

            void dispatch(const task_t& f)
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
                tasks_.emplace_front(f);
                awake_condition(lock, tasks_condition_);
            }

            bool wait()
            {
               return wait_(wait_all_mutex_, wait_all_condition_);
            }

            bool wait_one()
            {
                return wait_(wait_one_mutex_, wait_one_condition_);
            }

            bool start(const unsigned int& pool_size = std::thread::hardware_concurrency())
            {
                std::unique_lock < std::mutex > lock(mutex_);

                if (stop_)
                    return false;

                create_threads_(pool_size);

                return true;
            }

            bool stop()
            {
                std::unique_lock < std::mutex > lock(mutex_);

                std::unique_lock < std::mutex > lock_task(tasks_mutex_);

                if (stop_)
                    return false;

                stop_ = true;

                awake_condition(lock_task, tasks_condition_);

                remove_threads_();

                std::unique_lock < std::mutex > lock_all(wait_all_mutex_);

                std::unique_lock < std::mutex > lock_one(wait_one_mutex_);

                awake_condition(lock_all, wait_all_condition_);

                awake_condition(lock_one, wait_one_condition_);

                stop_ = false;

                return true;
            }

            void clear()
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
                tasks_.clear();
            }

            size_t size()
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
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
                std::unique_lock < std::mutex > lock_task(tasks_mutex_);

                tasks_condition_.wait(lock_task, [this]
                {
                    if (this->stop_)
                        return true;

                    std::unique_lock < std::mutex > lock_all(this->wait_all_mutex_);
                    std::unique_lock < std::mutex > lock_one(this->wait_one_mutex_);

                    if (this->tasks_.empty())
                    {
                        this->awake_condition(lock_all, this->wait_all_condition_);
                        this->awake_condition(lock_one, this->wait_one_condition_);
                        return false;
                    }
                    return true;
                });

                if (stop_)
                    return false;

                wait_one_condition_.notify_all();

                t = std::move(tasks_.front());
                tasks_.pop_front();
                return true;
            }

            void create_threads_(const unsigned int & pool_size)
            {
                for (unsigned int i = 0; i < pool_size; i++)
                    threads_.emplace_back(std::make_shared<std::thread>(std::bind(&BrinK::pool::thread::pool_func_, this)));
            }

            void remove_threads_()
            {
                std::for_each(threads_.begin(), threads_.end(), [](thread_ptr_t& td)
                { 
                    td->join();
                });
                threads_.clear();
            }

            bool stopped_()
            {
                std::unique_lock < std::mutex > lock(mutex_);
                return stop_ ? true : false;
            }

            void awake_condition(std::unique_lock < std::mutex >& mutex, std::condition_variable& cond)
            {
                cond.notify_all();
                mutex.unlock();
            }

            bool wait_(std::mutex& mutex, std::condition_variable& cond)
            {
                if (stopped_())
                    return false;

                std::unique_lock < std::mutex > lock_task(tasks_mutex_);

                if (tasks_.empty())
                    return false;

                std::unique_lock < std::mutex > lock_wait(mutex);
                lock_task.unlock();
                cond.wait(lock_wait);

                return true;
            }
        private:
            std::list < task_t >                                    tasks_;
            std::mutex                                              tasks_mutex_;
            std::condition_variable                                 tasks_condition_;

            std::list < thread_ptr_t >                              threads_;

            std::mutex                                              mutex_;
            std::atomic_bool                                        stop_;

            std::mutex                                              wait_all_mutex_;
            std::condition_variable                                 wait_all_condition_;

            std::mutex                                              wait_one_mutex_;
            std::condition_variable                                 wait_one_condition_;
        };

    }
}
#endif