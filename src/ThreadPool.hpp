#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop;

    public:
        ThreadPool() {
            size_t threadCount = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 2;
            
            for (size_t i = 0; i < threadCount; i++) {
                workers.emplace_back([this]() {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queueMutex);
                            this->condition.wait(lock, [this] {
                                return this->stop || !this->tasks.empty();
                            });

                            if (this->stop && this->tasks.empty()) return;

                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        void enqueue(std::function<void()> task) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                tasks.emplace(std::move(task));
            }
            condition.notify_one();
        }

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                stop = true;
            }
            condition.notify_all();
            for (size_t i = 0; i < workers.size(); i++) {
                if (workers[i].joinable()) workers[i].join();
            }
        }
};

#endif