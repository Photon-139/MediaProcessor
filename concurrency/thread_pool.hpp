#pragma once
#include "safe_queue.hpp"
#include <functional>
#include <thread>

class ThreadPool{
public:
    ThreadPool(int num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();
    void submit(std::function<void()> task);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    SafeQueue<std::function<void()>> queue_;
    std::vector<std::thread> threads_;
};