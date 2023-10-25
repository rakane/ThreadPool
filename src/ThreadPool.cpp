#include "ThreadPool.h"

// Constructor
Threads::ThreadPool::ThreadPool(const unsigned int numThreads)
    : poolMutex_(), condition_(), status_(PoolStatus::RUNNING), 
      threads_(), queueMutex_(), jobQueue_()
{    
    // Create threads
    for (int i = 0; i < numThreads; i++)
    {
        threads_.push_back(std::thread(std::bind(&ThreadPool::wait, this)));
    }
};

// Destructor
Threads::ThreadPool::~ThreadPool()
{
    // Shutdown thread pool
    shutdown();
};

// Shutdown thread pool
void Threads::ThreadPool::shutdown()
{    
    PoolStatus status = getStatus();

    // If shutdown() not already called
    if(status != PoolStatus::TERMINATED)
    {
        setStatus(PoolStatus::TERMINATED);

        // Notify all threads
        condition_.notify_all();

        // Join all threads
        for(unsigned int i = 0; i < threads_.size(); i++)
        {
            if(threads_[i].joinable())
            {
                threads_[i].join();
            }
        }
    }
}

void Threads::ThreadPool::clear()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    while(!jobQueue_.empty())
    {
        jobQueue_.pop();
    }
}

// Get number of queued jobs
size_t Threads::ThreadPool::numQueuedJobs() const
{
    return jobQueue_.size(); 
}

// Get number of threads in pool
unsigned int Threads::ThreadPool::numThreads() const
{
    return threads_.size();
}

// Get pool status
Threads::ThreadPool::PoolStatus Threads::ThreadPool::getStatus() const
{
    std::lock_guard<std::mutex> lock(poolMutex_);
    return status_;
}

// Set pool status
void Threads::ThreadPool::setStatus(PoolStatus status)
{
    std::lock_guard<std::mutex> lock(poolMutex_);
    status_ = status;
}

void Threads::ThreadPool::wait()
{   
    PoolStatus poolStatus = getStatus();

    while (poolStatus != PoolStatus::TERMINATED)
    {
        // Lock queue mutex
        std::unique_lock<std::mutex> lock(queueMutex_);

        // Wait for work, unlocks while waiting
        condition_.wait(lock, [this, &poolStatus]()
        {
            poolStatus = getStatus();

            return !jobQueue_.empty() || poolStatus == PoolStatus::TERMINATED;
        });

        // We own the lock now

        if(poolStatus != PoolStatus::TERMINATED)
        {
            if(!jobQueue_.empty())
            {
                // Get job from queue
                std::function<void()> jobFunc = jobQueue_.front();
                jobQueue_.pop();

                // Unlock lock
                lock.unlock();

                // Run job function
                jobFunc();
            } 
            else 
            {
                // Unlock lock
                lock.unlock();
            }
        }

        poolStatus = getStatus();
    }
};