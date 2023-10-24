#include "ThreadPool.h"

// Constructor
Threads::ThreadPool::ThreadPool(const unsigned int numThreads)
    : poolMutex_(), condition_(), status_(PoolStatus::RUNNING), 
      threads_(), queueMutex_(), jobQueue_()
{    
    for (int i = 0; i < numThreads; i++)
    {
        threads_.push_back(std::thread(std::bind(&ThreadPool::wait, this)));
    }
};

// Destructor
Threads::ThreadPool::~ThreadPool()
{
    shutdown();
};

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

// Shutdown thread pool
void Threads::ThreadPool::shutdown()
{    
    PoolStatus status = getStatus();

    // If shutdown() not already called
    if(status != PoolStatus::TERMINATE)
    {
        setStatus(PoolStatus::TERMINATE);

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

void Threads::ThreadPool::wait()
{   
    PoolStatus localStatus = getStatus();

    while (localStatus != PoolStatus::TERMINATE)
    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        // Wait for work
        condition_.wait(lock, [this, &localStatus]()
        {
            localStatus = getStatus();

            return !jobQueue_.empty() || localStatus == PoolStatus::TERMINATE;
        });

        if(localStatus != PoolStatus::TERMINATE)
        {
            // Get job from queue
            if(!jobQueue_.empty())
            {
                std::function<void()> jobFunc = jobQueue_.front();
                jobQueue_.pop();

                //Unlock lock
                lock.unlock();

                // Run job
                jobFunc();
            } 
            else 
            {
                // Unlock lock
                lock.unlock();
            }
        }

        localStatus = getStatus();
    }
};