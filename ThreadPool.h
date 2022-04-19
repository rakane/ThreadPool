#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace Threads {
    class ThreadPool {
        public:

            // Constructor
            ThreadPool(
                const unsigned int numThreads):
                numThreads_(numThreads)
            {    
                for (int i = 0; i < numThreads_; i++)
                {
                    threads_.push_back(std::thread([this] { wait(); }));
                }
            };

            // Destructor
            ~ThreadPool()
            {
                if(!terminate_)
                {
                    shutdown();
                }
            };

            // Add job to task queue
            template<typename F, typename ...Args>
            void addJob(F&& f, Args&&... args)
            {
                std::lock_guard<std::mutex> lock(poolMutex_);

                std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

                auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

                std::function<void()> wrapper = [task_ptr]() {
                    std::thread::id id = std::this_thread::get_id();
                    (*task_ptr)(); 
                };

                taskQueue_.push(wrapper);
                condition_.notify_one();
            };

            // Shutdown thread pool
            void shutdown()
            {    
                // If shutdown() not already called
                if(!terminate_)
                {
                    // Set terminate flag
                    {
                        std::lock_guard<std::mutex> lock(poolMutex_);
                        terminate_ = true;
                        condition_.notify_all();
                    }

                    // Join all threads
                    for(unsigned int i = 0; i < numThreads_; i++)
                    {
                        if(threads_[i].joinable())
                        {
                            threads_[i].join();
                        }
                    }
                }
            }

        private:
            void wait()
            {   
                while (!terminate_)
                {
                    {
                        std::unique_lock<std::mutex> lock(poolMutex_);

                        // Wait for work
                        condition_.wait(lock, [this](){
                            return !taskQueue_.empty() || terminate_;
                        });
                        
                        // Get job from queue
                        if(!taskQueue_.empty())
                        {
                            std::function<void()> job = taskQueue_.front();
                            taskQueue_.pop();

                            //Unlock lock
                            lock.unlock();

                            // Run job
                            job();
                        }
                    }
                }
            };

            // Pool Mutex
            std::mutex poolMutex_;

            // Pool condition variable
            std::condition_variable condition_;

            // Pool status
            bool terminate_;

            // Thread pool
            unsigned int numThreads_;
            std::vector<std::thread> threads_;

            // Job queue
            std::queue<std::function<void()>> taskQueue_;
    };
}

#endif