#pragma once
#include <queue>
#include <thread>
#include <mutex> // pthdowhile 创建宏 为什么要加\read_mutex_t
#include <condition_variable> // pthread_condition_t

// 异步写日志的日志队列
template<typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志queue 
    void Push(const T &data)
    {
        //当lock_guard的析构函数在作用域结束时自动释放互斥锁，确保其他线程可以安全地访问队列。
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }

    // 一个线程读日志queue，写日志文件
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
            //先unlock之前获得的mutex,然后阻塞当前的执行线程
            //把当前线程添加到等待线程列表中，该线程会持续 block 直到被 notify_all() 或 notify_one() 唤醒。
            //被唤醒后，该thread会重新获取mutex，获取到mutex后执行后面的动作。
        }

        T data = m_queue.front();
        //不能返回一个局部变量的引用，因为局部变量在函数返回后会被销毁，这会导致返回的引用变得无效。
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable; //条件变量
};