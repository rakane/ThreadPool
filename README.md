# Thread Pool

A simple C++ thread pool, offering a simple interface to run jobs in parallel on a configurable number of threads.

## Usage

A thread pool can be created with a given number of threads as follows:

```c++
ThreadPool pool = ThreadPool(4);
```

Jobs can be added to the thread pool as follows:

```c++
void printThing()
{
    std::cout << "Thing" << std::endl;
}

pool.addJob(printThing);
```

Arguments can be passed to the job as follows:

```c++
void addThing(int a, int b)
{
    std::cout << "add(): " << a << " + " << b << " = " << a + b << "\n";
}

pool.addJob(addThing, 1, 2);
```

Object methods can be passed as jobs as follows:

```c++
MyClass myClass;

std::function<void()> f = std::bind(&MyClass::func1, &myClass);
pool.addJob(f);

std::function<void(unsigned int)> f2 = std::bind(&MyClass::func2, &myClass, std::placeholders::_1);
pool.addJob(f2, 10);
```

A std::future is returned when a job is added to the thread pool. This can be used to wait for the job to complete and to retrieve the return value of the job. If the pool is shutdown or cleared before the job is executed, the future will throw a std::future_error exception when the return value is requested. If the job is added after the pool is already shutdown, an invalid std::future is returned. If get() is called on the future in this case, a std::future_error exception will be thrown:

```c++
// Return future from job
std::future<int> future = pool.addJob(addThingResult, 1, 2);

try
{
    if(future.valid())
    {
        int result = future.get();
        std::cout << "Result: " << result << std::endl;
    }
}
catch(const std::future_error& e)
{
    // Future invalid, job not completed
    std::cout << "Exception: " << e.what() << std::endl;
}
```
