#include "thread_pool.hpp"
#include <mutex>
#include <spdlog/spdlog.h>

ThreadPool::ThreadPool(int num_threads){
    for(int i = 0; i<num_threads; ++i){
        threads_.emplace_back([this, i](){
            // spdlog::info("Worker thread {} started, id: {}", i, std::this_thread::get_id());
            while(true){
                auto task = queue_.pop();
                if (!task) break;
                try {
                    (*task)();
                } catch (const std::exception& e) {
                    spdlog::error("Task exception: {}", e.what());
                }catch(...){
                    spdlog::error("Task exception: Unknown exception caught.");
                }
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    queue_.shutdown();
    for(auto& t : threads_){
        if(t.joinable()){
            // spdlog::info("Joing thread with id: {}", t.get_id());
            t.join();
        }
    }
}

void ThreadPool::submit(std::move_only_function<void()> task){
    queue_.push(std::move(task));
}