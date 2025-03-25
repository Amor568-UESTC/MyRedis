#pragma once

#include<deque>
#include<functional>
#include<thread>
#include<memory>
#include<future>
#include<atomic>
#include<mutex>
#include<condition_variable>

class ThreadPool final
{
private:
    ThreadPool();

    void CreateWorker_();
    void WorkerRoutine_();
    void MonitorRoutine_();

    std::thread monitor_;
    std::atomic<unsigned> maxIdleThread_;
    std::atomic<unsigned> pendingStopSignal_;

    static __thread bool working_;
    std::deque<std::thread> workers_;

    std::mutex mutex_;
    std::condition_variable cond_;
    unsigned waiters_;
    bool shutdown_;
    std::deque<std::function<void()>> tasks_;
    static const int KMaxThreads=256;

public:
    ~ThreadPool();
    ThreadPool(const ThreadPool&)=delete;
    void operator=(const ThreadPool&)=delete;

    static ThreadPool& Instance();

    template<typename F,typename... Args>
    auto ExecuteTask(F&& f,Args&&... args)->std::future<typename std::result_of<F (Args...)>::type>;

    void JoinAll();
    void SetMaxIdeleThread(unsigned int m);

};

template<typename F,typename... Args>
auto ThreadPool::ExecuteTask(F&& f,Args&& ... args)->std::future<typename std::result_of<F (Args...)>::type>
{
    using resultType=typename std::result_of<F (Args ...)>::type;
    auto task=std::make_shared<std::package_tast<resultType()>>
    {
        std::unique_lock<std::mutex> guard(mutex_);
        if(shutdown_)
            return std::future<resultType>();
        task_.emplace_back([=]() {(*task)();});
        if(waiters_==0)
            CreateWorker_();
        cond_.notify_one();
    }
    return task->get_future();
}