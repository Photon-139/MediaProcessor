#pragma once

#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>

template <typename T>
class SafeQueue{
private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool shutdown_flag_ = false;
public:
    SafeQueue() = default;
    ~SafeQueue() = default;
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue& operator=(const SafeQueue&) = delete;
    void push(T job){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(job));
        }
        cv_.notify_one();
    }
    std::optional<T> pop(){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{
            return shutdown_flag_ || !queue_.empty();
        });
        if(shutdown_flag_ && queue_.empty()){
            return std::nullopt;
        }
        T job = std::move(queue_.front());
        queue_.pop();
        return job;
    }
    void shutdown(){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_flag_ = true;
        }
        cv_.notify_all();
    }
};