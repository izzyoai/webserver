#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<functional>
#include<vector>
#include<iostream>


class ThreadPool{

public:

    ThreadPool(int thread_num):stop(false)
    {
        for(int i = 0;i < thread_num;i++)
        {
            //每个线程执行的方法
            threads.emplace_back([this]()
            {
                while(true)
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    //当任务不为空或停止线程，则不再等待
                    condition.wait(lock,[this](){
                        return !tasks.empty() || stop;
                    });
                    //如果停止，则结束线程
                    if(tasks.empty() && stop)
                    {
                        return;
                    }
                    //转移方法执行
                    std::function<void()> task(move(tasks.front()));
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }
    ~ThreadPool()
    {
        //局部代码，设置stop为true自动解锁
        {
            std::unique_lock lock(mtx);
            stop = true;
        }
        //通知所有线程完成任务
        condition.notify_all();
        //等待各个线程结束
        for(auto& t : threads)
        {
            t.join();
        }
    }

    template<class F,class... Args>
    void enqueue(F&& f,Args&&... args)
    {
        //将任务绑定到task
        std::function<void()> task = 
        std::bind(std::forward<F>(f),std::forward<Args>(args)...);
        //插入任务
        {
            std::unique_lock lock(mtx);
            tasks.emplace(move(task));
        }
        //通知一个线程进行执行
        condition.notify_one();
    }

private:
    
    std::vector<std::thread> threads;

    std::queue<std::function<void()>> tasks;

    std::condition_variable condition;

    std::mutex mtx;

    bool stop;

};


#endif