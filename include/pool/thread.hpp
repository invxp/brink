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
                stop_flag_ = true;
                can_start_ = true;
            }
            virtual ~thread()
            {
                stop();
                cancel();
            }

        public:
            void post(const task_t& f)
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
                tasks_.push_back(f);
                awake_thread_();
            }

            void dispatch(const task_t& f)
            {
                std::unique_lock < std::mutex > lock(tasks_mutex_);
                tasks_.push_front(f);
                awake_thread_();
            }

            bool wait()
            {
                if (stopped_())
                    return false;

                std::unique_lock < std::mutex > lock_task(tasks_mutex_);
                std::unique_lock < std::mutex > lock_wait(wait_all_mutex_);
                awake_thread_(true);
                lock_task.unlock();
                wait_all_condition_.wait(lock_wait);

                return true;
            }

            bool wait_one()
            {
                if (stopped_())
                    return false;

                std::unique_lock < std::mutex > lock_task(tasks_mutex_);
                std::unique_lock < std::mutex > lock_wait(wait_one_mutex_);
                awake_thread_(true);
                lock_task.unlock();
                wait_one_condition_.wait(lock_wait);

                return true;
            }

            void stop()
            {
                std::unique_lock < std::mutex > lock_start(can_start_mutex_);
                std::unique_lock < std::mutex > lock_stop(stop_mutex_);
                std::unique_lock < std::mutex > lock_task(tasks_mutex_);
                std::unique_lock < std::mutex > lock_all(wait_all_mutex_);
                std::unique_lock < std::mutex > lock_one(wait_one_mutex_);

                if (stop_flag_)
                    return;

                stop_flag_ = true;

                awake_thread_(true);
                wait_all_condition_.notify_all();
                wait_one_condition_.notify_all();

                lock_task.unlock();
                lock_all.unlock();
                lock_one.unlock();
                lock_stop.unlock();

                std::for_each(threads_.begin(), threads_.end(), [this](thread_ptr_t& thread){ thread->join(); });
                threads_.clear();
                can_start_ = true;
            }

            bool start(const unsigned int& pool_size = std::thread::hardware_concurrency())
            {
                std::unique_lock < std::mutex > lock_start(can_start_mutex_);

                if (!can_start_)
                    return false;

                can_start_ = false;

                std::unique_lock < std::mutex > lock_stop(stop_mutex_);

                stop_flag_ = false;

                run_(pool_size);

                return true;
            }

            void cancel()
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
                do
                {
                    std::unique_lock < std::mutex > lock_stop(stop_mutex_);
                    if (stop_flag_)
                        break;

                    std::unique_lock < std::mutex > lock_task(tasks_mutex_);
                    std::unique_lock < std::mutex > lock_all(wait_all_mutex_);
                    std::unique_lock < std::mutex > lock_one(wait_one_mutex_);

                    if (tasks_.empty())
                    {
                        wait_all_condition_.notify_all();
                        lock_all.unlock();
                        wait_one_condition_.notify_all();
                        lock_one.unlock();
                        lock_stop.unlock();
                        tasks_condition_.wait(lock_task);
                        continue;
                    }

                    tasks_.front()();
                    tasks_.pop_front();
                    wait_one_condition_.notify_all();
                    lock_one.unlock();

                    std::this_thread::yield();

                } while (!stop_flag_);
            }

            void awake_thread_(const bool& all = false)
            {
                all ? tasks_condition_.notify_all() : tasks_condition_.notify_one();
            }

            void run_(const unsigned int & pool_size)
            {
                for (unsigned int i = 0; i < pool_size; i++)
                    threads_.emplace_back(std::make_shared<std::thread>(std::bind(&BrinK::pool::thread::pool_func_, this)));
            }

            bool stopped_()
            {
                std::unique_lock < std::mutex > lock_stop(stop_mutex_);
                return stop_flag_ ? true : false;
            }
        private:
            std::list < task_t >                                    tasks_;
            std::mutex                                              tasks_mutex_;
            std::condition_variable                                 tasks_condition_;

            std::list < thread_ptr_t >                              threads_;

            std::mutex                                              stop_mutex_;
            volatile std::atomic_bool                               stop_flag_;

            std::mutex                                              can_start_mutex_;
            volatile std::atomic_bool                               can_start_;

            std::mutex                                              wait_all_mutex_;
            std::condition_variable                                 wait_all_condition_;

            std::mutex                                              wait_one_mutex_;
            std::condition_variable                                 wait_one_condition_;
        };

    }
}
#endif