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
        ThreadPool(const unsigned int numThreads);
        ~ThreadPool();

        // Shutdown thread pool
        void shutdown();

        // Get number of threads in pool
        size_t numQueuedJobs() const;

        unsigned int numThreads() const;

        // Add job to task queue, returning future
        template<typename F, typename ...Args>
        std::future<std::invoke_result_t<F, Args...>> addJob(F&& f, Args&&... args)
        {
            // Bind arguments to function
            std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

            // Create packaged task for job
            std::shared_ptr<std::packaged_task<decltype(f(args...))()>> taskPtr 
                = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

            // Get future from packaged task
            std::future<std::invoke_result_t<F, Args...>> future = taskPtr->get_future();

            // Wrap in void function
            std::function<void()> wrapperFunc = [taskPtr]() {
                (*taskPtr)();
            };

            // Add job to queue
            {
                std::lock_guard<std::mutex> lock(poolMutex_);
                jobQueue_.push(wrapperFunc);
            }

            // Notify one thread
            condition_.notify_one();

            return future;
        };

    private:
        enum PoolStatus
        {
            RUNNING = 0,
            TERMINATE = 1
        };

        PoolStatus getStatus() const;
        void setStatus(PoolStatus status);

        // Wait for work
        void wait();

        // Pool Mutex
        mutable std::mutex poolMutex_;

        // Pool condition variable
        std::condition_variable condition_;

        // Pool status
        PoolStatus status_;

        // Thread pool
        std::vector<std::thread> threads_;

        // Job queue
        mutable std::mutex queueMutex_;
        std::queue<std::function<void()>> jobQueue_;
    };
}

#endif