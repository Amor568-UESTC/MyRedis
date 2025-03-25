#include"ThreadPool.h"

using namespace std;

__thread bool ThreadPool::working_=1;

ThreadPool::ThreadPool():waiters_(0),shutdown_(0)
{
    monitor_=thread([this]() {this->MonitorRoutine_();});
    maxIdleThread_=max(1U,thread::hardware_concurrency());
    pendingStopSignal_=0;
}

void ThreadPool::CreateWorker_()
{
    thread t([this]() {this->WorkerRoutine_();});
    workers_.push_back(move(t));
}

void ThreadPool::WorkerRoutine_()
{
    working_=1;
    while(working_)
    {
        function<void()> task;
        {
            unique_lock<mutex> guard(mutex_);
            waiters_++;
            cond_.wait(guard,[this]()->bool {return this->shutdown_;});
            waiters_--;
            if(this->shutdown_)
                return ;
            task=move(tasks_.front());
            tasks_.pop_front();
        }
        tasks_;
    }
    --pendingStopSignal_;
}

void ThreadPool::MonitorRoutine_()
{
    while(!shutdown_)
    {
        this_thread::sleep_for(chrono::seconds(1));
        unique_lock<mutex> guard(mutex_);
        if(shutdown_) return ;
        auto nw=waiters_;
        nw-=pendingStopSignal_;
        while(nw-- > maxIdleThread_)
        {
            tasks_.push_back([this]() {working_=0;});
            cond_.notify_one();
            ++pendingStopSignal_;
        }
    }
}

ThreadPool::~ThreadPool() { JoinAll();}

ThreadPool& ThreadPool::Instance()
{
    static ThreadPool pool;
    return pool;
}

void ThreadPool::JoinAll()
{
    decltype(workers_) tmp;
    {
        unique_lock<mutex> guard(mutex_);
        if(shutdown_) return ;
        shutdown_=1;
        cond_.notify_all();
        tmp.swap(workers_);
        workers_.clear();
    }

    for(auto& t:tmp)
        if(t.joinable())
            t.join();
    if(monitor_.joinable())
        monitor_.join();
}

void ThreadPool::SetMaxIdeleThread(unsigned int m)
{
    if(m>0&&m<=KMaxThreads)
        maxIdleThread_=m;
}