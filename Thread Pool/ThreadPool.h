#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include "SafeQueue.h"
#include <iostream>

using namespace std;

class ThreadPool {
	vector<thread> workers; // Vector to hold worker threads
	TSQueue<function<void()>> tasks; // Thread-safe queue for tasks
	atomic<bool> stop; // Flag to signal stopping the thread pool
public:	
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; i++) {
			workers.emplace_back([this] {
				while (!stop) {
					function<void()> task;
					try {
						task = tasks.pop(); // Get a task from the queue
						task(); // Execute the task
					} catch (const exception& e) {
						// Handle any exceptions thrown by tasks
						cerr << "Task error: " << e.what() << endl;
					}
				}
			});
		}
	}
	template<class F, class... Args>
	void enqueue(F&& f, Args&&... args) {
		tasks.push([f, args...]() { f(args...); }); // Add a new task to the queue
	}
	~ThreadPool() {
		stop = true; // Signal all threads to stop
		for (thread &worker : workers) {
			if (worker.joinable()) {
				worker.join(); // Wait for all threads to finish
			}
		}
	}
};