//
// Created by Utsav Lal on 10/26/24.
//

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T, typename Compare>
class ThreadSafePriorityQueue {
private:
    std::priority_queue<T, std::vector<T>, Compare> pq;
    mutable std::mutex mtx;

public:
    // Insert an element into the priority queue
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        pq.push(std::move(item));
    }

    // Retrieve and remove the highest-priority element from the queue
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        if (pq.empty()) return false;

        item = pq.top();
        pq.pop();
        return true;
    }

    // Check if the queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return pq.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return pq.size();
    }

    bool peek(T& item) const {
        std::lock_guard<std::mutex> lock(mtx);
        if (pq.empty()) return false;

        item = pq.top();
        return true;
    }
};
