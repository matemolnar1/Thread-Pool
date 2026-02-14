#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <future>   // For async results
#include <memory>   // For shared_ptr

using namespace std;

// This prevents threads from fighting over the same data.
template <typename T>
class SafeQueue {
private:
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_cond;
    bool m_stop = false; // "Poison pill" to stop the queue

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

        // Wait until queue is not empty OR shutdown is requested
        m_cond.wait(lock, [this]() {
            return !m_queue.empty() || m_stop;
            });

        // If stopped and empty, return false
        if (m_queue.empty() && m_stop) {
            return false;
        }

        item = move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void shutdown() {
        {
            unique_lock<mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cond.notify_all(); // Wake up everyone so they can check m_stop and exit
    }
};
// The manager that hires workers and assigns tasks.
class ThreadPool {
    vector<thread> workers;
    SafeQueue<function<void()>> tasks;

public:
    // Constructor: Launches the worker threads
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    function<void()> task;

                    // If pop returns false, the pool is shutting down
                    if (!tasks.pop(task)) {
                        return;
                    }

                    task(); // Execute the task
                }
                });
        }
    }

    // Destructor: Cleans up threads gently
    ~ThreadPool() {
        tasks.shutdown(); // Tell threads to stop accepting work
        for (thread& worker : workers) {
            if (worker.joinable()) {
                worker.join(); // Wait for them to finish current tasks
            }
        }
    }

    
    // 1. Takes any function F and arguments Args
    // 2. Returns a future (ticket) for the result
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> future<typename result_of<F(Args...)>::type>
    {
        // Deduce return type
        using return_type = typename result_of<F(Args...)>::type;

        // Create a packaged_task (wraps the function so it can return a future)
        // We use shared_ptr because std::function requires copyable objects
        auto task = make_shared<packaged_task<return_type()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );

        future<return_type> res = task->get_future();

        // Push a lambda into the queue that executes the task
        tasks.push([task]() { (*task)(); });

        return res;
    }
};


int main() {
    // 1. Create a pool with 4 threads 
    ThreadPool pool(4);

    cout << "1. Submitting simple tasks..." << endl;

    // Submit 8 tasks
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            // Simulate work
            this_thread::sleep_for(chrono::milliseconds(100));

            // Print (mutex needed for clean output, but skipped for simplicity)
            cout << "Task " << i << " finished by thread " << this_thread::get_id() << endl;
            });
    }

    cout << "2. Submitting a calculation task (Square of 10)..." << endl;

    // Submit a task that returns a value (int)
    auto future_result = pool.enqueue([](int x) {
        cout << "   Calculating square..." << endl;
        this_thread::sleep_for(chrono::seconds(2)); // Heavy math simulation
        return x * x;
        }, 10);

    cout << "   Main thread is free to do other things..." << endl;

    // .get() waits for the result if it's not ready yet
    int result = future_result.get();

    cout << "   Result received: " << result << endl;

    return 0;
}