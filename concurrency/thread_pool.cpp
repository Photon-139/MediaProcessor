#include "thread_pool.hpp"
#include <iostream>
#include <mutex>

static std::mutex g_log_mtx;

ThreadPool::ThreadPool(int num_threads){
    for(int i = 0; i<num_threads; ++i){
        threads_.emplace_back([this](){
            while(true){
                auto task = queue_.pop();
                if (!task) break;
                try {
                    (*task)();
                } catch (const std::exception& e) {
                    std::lock_guard<std::mutex> lock(g_log_mtx);
                    std::cerr << "Task exception: " << e.what() << std::endl;
                }catch(...){
                    std::lock_guard<std::mutex> lock(g_log_mtx);
                    std::cerr << "Task exception: Unknown exception caught." << std::endl;
                }
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    queue_.shutdown();
    for(auto& t : threads_){
        t.join();
    }
}

void ThreadPool::submit(std::function<void()> task){
    queue_.push(std::move(task));
}