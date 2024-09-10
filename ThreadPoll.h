#include <SFML/Graphics.hpp>
#include<mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <thread>
#include<vector>
#include<iostream>

using namespace sf;

class ThreadPool {
public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this] { return !tasks_.empty() || stop_; });
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        task = tasks_.front();
                        tasks_.pop();
                    }
                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    template <typename Func>
    void enqueue(Func&& func) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace(std::forward<Func>(func));
        }
        condition_.notify_one();
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
};