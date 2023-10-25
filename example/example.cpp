#include "ThreadPool.h"

#include <thread>
#include <iostream>
#include <unistd.h>

class MyClass
{
public:
    MyClass() {}
    ~MyClass() {}
    void func1() { }
    void func2(unsigned int num) { }
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

void myWait(unsigned int microseconds)
{
    usleep(microseconds);
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

    std::function<void()> f = std::bind(&MyClass::func1, &myClass);
    pool.addJob(f);

    std::function<void(unsigned int)> f2 = std::bind(&MyClass::func2, &myClass, std::placeholders::_1);
    pool.addJob(f2, 10);

    // Return future from job
    std::future<int> future = pool.addJob(addThingResult, 1, 2);

    // If pool is shutdown or cleared prior to job completion, future will be invalid
    try 
    {
        int result = future.get();
        std::cout << "Result: " << result << std::endl;
    }
    catch(const std::future_error& e)
    {
        // Future invalid, job not completed
        std::cout << "Exception: " << e.what() << std::endl;
    }

    // Can query number of jobs in queue
    while(pool.numQueuedJobs() > 0)
    {
        std::cout << "Waiting for jobs to complete..." << std::endl;
        usleep(100000);
    }

    // Terminating the pool will stop all further queuing and processing of jobs
    pool.shutdown();

    // Attempting to queue a job after the pool is terminated will return an invalid future,
    // and the job will not be queued
    future = pool.addJob(addThingResult, 1, 2);
    std::cout << "Future valid: " << future.valid() << std::endl;

    return 0;
}