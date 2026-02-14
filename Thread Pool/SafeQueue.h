#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

template <typename T>
class TSQueue {
private:
    queue<T> m_queue;  // Underlying queue
    mutex m_mutex; // mutex for thread synchronization
    condition_variable m_cond; // Condition variable for signaling

public:
    // Pushes an element to the queue
    void push(T item)
    {
        unique_lock<mutex> lock(m_mutex); // Acquire lock
        m_queue.push(item); // Add item
		m_cond.notify_one(); // Notify one waiting thread
    }

    // Pops an element off the queue
    T pop()
    {
		unique_lock<mutex> lock(m_mutex); // Acquire lock
        m_cond.wait(lock, [this]() { return !m_queue.empty(); }); // Wait until the queue is not empty
        T item = m_queue.front();  // retrieve item
        m_queue.pop();

       return item;  // return item
    }
};
