#ifndef __AJOB_HPP__
#define __AJOB_HPP__

#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

#include "Query.hpp"
#include "ThreadPool.hpp"

class BaseJob {
    protected:
        static ThreadPool threadPool;
};


template<typename... Args>
class AJob : public BaseJob {

    private:
        Query<Args...> query;
        std::vector<std::thread> threads;
        size_t threadCount = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 2;
        std::mutex mtx;

        std::atomic<int> activeTasks{0};
        std::mutex waitMutex;
        std::condition_variable waitCondition;

    protected:
        AJob() { query = Query<Args...>(); };
        void safeLock() { mtx.lock(); }
        void safeUnlock() { mtx.unlock(); }

        template<typename T>
        std::pair<char*, size_t> getComponentDataInIndex(size_t index) {
            return query.template getComponentDataInIndex<T>(index);
        }

    public:
        virtual ~AJob() {};
        virtual void execute(size_t index) = 0;
        
        void scheduleParallel() {
            size_t totalItems = query.getTotalChunkCount();
            if (totalItems == 0) return;

            size_t itemsPerThread = (totalItems + threadCount - 1) / threadCount;
            activeTasks = 0;

            for (size_t i = 0; i < threadCount; i++) {
                size_t startIdx = i * itemsPerThread;
                size_t endIdx = std::min(startIdx + itemsPerThread, totalItems);

                if (startIdx >= totalItems) break;

                activeTasks++;
                
                threadPool.enqueue([this, startIdx, endIdx]() {
                    for (size_t j = startIdx; j < endIdx; j++) {
                        this->execute(j);
                    }
                    
                    if (--activeTasks == 0) {
                        std::lock_guard<std::mutex> lock(waitMutex);
                        waitCondition.notify_all();
                    }
                });
            }
            std::unique_lock<std::mutex> lock(waitMutex);
            waitCondition.wait(lock, [this] { return activeTasks == 0; });
        }

        void schedule() {
            for (size_t i = 0; i < query.getTotalChunkCount(); ++i) {
                this->execute(this->query.getComponentDataInIndex(i));
        }
}

};

#endif