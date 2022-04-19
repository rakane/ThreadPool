#include "ThreadPool.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <time.h>

void simpleCalc1() 
{
    usleep(10000);
    std::cout << "simpleCalc1(): " << rand() % (1000 - 1 + 1) + 1 << "\n";
}

void add(int a, int b)
{
    usleep(100000);
    std::cout << "add(): " << a << " + " << b << " = " << a + b << "\n";
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    const unsigned int numThreads = std::thread::hardware_concurrency();

    Threads::ThreadPool* pool = 
        new Threads::ThreadPool(numThreads);

    for(int i = 0; i < 10; i++)
    {
        pool->addJob(add, 10, 20);
    }

    for(int i = 0; i < 10; i++)
    {
        pool->addJob(simpleCalc1);
    }

    delete pool;

    return 0;
}