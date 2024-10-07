//
// Created by Utsav Lal on 10/6/24.
//

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <queue>
#include <thread>


class ThreadPool {
public:
    ThreadPool(size_t numThreads);

    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()> > tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void workerThread();
};


#endif //THREADPOOL_HPP
