//
// Created by Utsav Lal on 9/21/24.
//

#ifndef SAFE_QUEUE_HPP
#define SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

/**
 *Inspired by https://stackoverflow.com/a/16075550
 *
 * @brief A thread-safe queue implementation.
 *
 * This class provides a thread-safe wrapper around a standard queue,
 * ensuring that enqueue and dequeue operations are safe to call from
 * multiple threads concurrently.
 *
 * @tparam T The type of elements stored in the queue.
 */
template<class T>
class SafeQueue {
public:
    /**
     * @brief Constructs an empty SafeQueue.
     */
    SafeQueue(): q() {
    }

    /**
     * @brief Default destructor.
     */
    ~SafeQueue() = default;

    /**
     * @brief Adds an element to the queue.
     *
     * This method is thread-safe and will notify one waiting thread
     * that an element has been added.
     *
     * @param t The element to add to the queue.
     */
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    /**
     * @brief Removes and returns the front element of the queue.
     *
     * If the queue is empty, this method will block until an element
     * is available.
     *
     * @return The front element of the queue.
     */
    T dequeue() {
        std::unique_lock<std::mutex> lock(m);
        c.wait(lock, [&]{ return !q.empty(); });
        T val = q.front();
        q.pop();
        return val;
    }

    /**
     * @brief Checks if the queue is not empty.
     *
     * This method is thread-safe.
     *
     * @return True if the queue is not empty, false otherwise.
     */
    bool notEmpty() const {
        std::lock_guard<std::mutex> lock(m);
        return !q.empty();
    }

private:
    std::queue<T> q; ///< The underlying queue storing the elements.
    mutable std::mutex m; ///< Mutex for synchronizing access to the queue.
    std::condition_variable c; ///< Condition variable for blocking dequeue operations.
};

#endif //SAFE_QUEUE_HPP