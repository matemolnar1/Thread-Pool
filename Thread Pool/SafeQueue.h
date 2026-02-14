#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

template <typename T>
class SafeQueue {
private:
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_cond;
    bool m_stop = false; // Flag to stop the queue

public:
    void push(T item) {
        {
            unique_lock<mutex> lock(m_mutex);
            m_queue.push(move(item));
        }
        m_cond.notify_one();
    }

    // Returns false if queue is stopped and empty
    bool pop(T& item) {
        unique_lock<mutex> lock(m_mutex);

        // Wait until queue is not empty OR stop is requested
        m_cond.wait(lock, [this]() {
            return !m_queue.empty() || m_stop;
            });

        // If stopped and empty, return false so the worker can exit
        if (m_queue.empty() && m_stop) {
            return false;
        }

        item = move(m_queue.front());
        m_queue.pop();
        return true;
    }

    // Wakes up all waiting threads so they can exit gracefully
    void shutdown() {
        {
           unique_lock<mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cond.notify_all();
    }
};