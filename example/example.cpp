#include "ThreadPool.h"

#include <thread>
#include <iostream>
#include <unistd.h>

class MyClass
{
public:
    MyClass(): count_(0) {}
    ~MyClass() {}

    void increment() 
    {   
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "increment(): " << count_ << " -> " << count_ + 1 << "\n";
        count_++; 
    }

    void add(unsigned int num) 
    { 
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "add(): " << count_ << " + " << num << " = " << count_ + num << "\n";
        count_ += num;
    }

private:
    // User must implement thread safety on objects used in thread pool
    mutable std::mutex mutex_;

    unsigned int count_;
};

void printThing() 
{
    std::cout << "printThing() called" << "\n";
}

void addThing(int a, int b)
{
    std::cout << "add(): " << a << " + " << b << " = " << a + b << "\n";
}

int addThingResult(int a, int b)
{
    std::cout << "addThingResult(): " << a << " + " << b << " = " << a + b << "\n";
    return a + b;
}

int main(int argc, char** argv)
{
    // Construct pool with 2 threads, threads started immediately
    const unsigned int NUM_THREADS = 2;
    Threads::ThreadPool pool(NUM_THREADS);

    // Add job to queue
    pool.addJob(printThing);

    // Add job to queue with arguments
    pool.addJob(addThing, 10, 20);

    // Invoke methods on objects using std::bind and std::function
    MyClass myClass;

    std::function<void()> f = std::bind(&MyClass::increment, &myClass);
    pool.addJob(f);

    std::function<void(unsigned int)> f2 = std::bind(&MyClass::add, &myClass, std::placeholders::_1);
    pool.addJob(f2, 10);

    // Return future from job, print result
    std::future<int> future = pool.addJob(addThingResult, 1, 2);
    int result = future.get();
    std::cout << "Result: " << result << std::endl;

    // Can query number of jobs in queue
    while(pool.numQueuedJobs() > 0)
    {
        std::cout << "Waiting for jobs to complete..." << std::endl;
        usleep(100000);
    }

    // Terminating the pool will stop all further queuing and processing of jobs
    pool.shutdown();

    return 0;
}