# High-Performance C++ Thread Pool

A lightweight, header-only Thread Pool library implementation in modern C++. 

This project demonstrates the creation of a high-complexity system component that manages concurrent execution of tasks using a fixed number of worker threads. It solves the overhead of creating/destroying threads for every task by maintaining a persistent pool of workers.

## Key Features

* **Thread-Safe Task Queue:** Implemented a custom `SafeQueue` using `std::mutex` and `std::condition_variable` to prevent data races.
* **Asynchronous Results:** Uses `std::future` and `std::packaged_task` to return values from background threads to the main thread.
* **Variadic Templates:** The `enqueue` function uses variadic templates and perfect forwarding (`std::forward`) to accept *any* function with *any* number of arguments.
* **RAII Compliant:** The destructor ensures all threads are joined and resources are released gracefully upon shutdown.
* **Non-Blocking:** Workers sleep when the queue is empty, ensuring 0% CPU usage when idle.

## Technical Concepts Applied

This project focuses on system-level C++ programming:
1.  **Concurrency & Synchronization:** Handling race conditions and thread signaling.
2.  **Memory Management:** Using `std::unique_lock` and `std::shared_ptr` for safe resource handling.
3.  **Template Metaprogramming:** Deducing return types automatically using `std::result_of`.
4.  **Move Semantics:** Optimizing queue operations by moving tasks instead of copying them.

## Getting Started

### Prerequisites
* C++ Compiler (GCC, Clang, or MSVC) supporting C++11 or later.

### Compilation
Since this uses standard threading libraries, link against `pthread` (on Linux/Mac):

```bash
g++ -std=c++14 -pthread main.cpp -o threadpool
./threadpool
