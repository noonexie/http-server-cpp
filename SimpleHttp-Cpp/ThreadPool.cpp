#include <thread>
#include <iostream>
#include "ThreadPool.h"
using namespace std;

const int NUMBER = 2;

ThreadPool::ThreadPool(int min, int max)
{
    do
    {
        m_shutdown = false;
        m_minNum = min;
        m_maxNum = max;

        // 管理者线程
        m_managerId = thread(&ThreadPool::manager, this);

        // 工作者线程
        m_workIds.resize(m_maxNum);
        for (int i = 0; i < m_minNum; i++)
        {
            m_workIds[i] = thread(&ThreadPool::worker, this);
        }
        return;
    } while (0);
}

ThreadPool::~ThreadPool()
{
    m_shutdown = true;
    // 阻塞回收管理者线程
    if (m_managerId.joinable())
        m_managerId.join();
    // 唤醒阻塞的消费者线程
    m_condQueue.notify_all();
    for (int i = 0; i < m_maxNum; ++i)
    {
        if (m_workIds[i].joinable())
            m_workIds[i].join();
    }
}

void ThreadPool::addTask(Task t)
{
    unique_lock<mutex> lk(m_mutexPool);
    if (m_shutdown)
    {
        return;
    }
    // 添加任务
    m_taskQ.push(t);
    m_condQueue.notify_all();
}

void ThreadPool::addTask(callback f, void *a)
{
    unique_lock<mutex> lk(m_mutexPool);
    if (m_shutdown)
    {
        return;
    }
    // 添加任务
    m_taskQ.push(Task(f, a));
    m_condQueue.notify_all();
}

void ThreadPool::manager()
{
    // ThreadPool *pool = static_cast<ThreadPool *>(args);
    // 管理者线程也需要不停的监视线程池队列和工作者线程
    while (!m_shutdown)
    {
        // 每隔三秒检测一次
        this_thread::sleep_for(chrono::seconds(3));
        unique_lock<mutex> lk(m_mutexPool);
        int taskNum = m_taskQ.size();
        int liveNum = m_liveNum;
        int busyNum = m_busyNum;
        lk.unlock();

        // 添加线程
        if (taskNum > liveNum && liveNum < m_maxNum)
        {
            lk.lock();
            // 用于计数，添加的线程个数
            int count = 0;
            // 添加线程
            for (int i = 0; i < m_maxNum && count < NUMBER && m_liveNum < m_maxNum; ++i)
            {
                // 判断当前线程ID,用来存储创建的线程ID
                if (m_workIds[i].get_id() == thread::id())
                {
                    cout << "Create a new thread..." << endl;
                    m_workIds[i] = thread(&ThreadPool::worker, this);
                    // 线程创建完毕
                    count++;
                    m_liveNum++;
                }
            }
            lk.unlock();
        }

        // 销毁线程:当前存活的线程太多了,工作的线程太少了
        // 忙的线程*2 < 存活的线程数 && 存活的线程数 >  最小的线程数
        if (busyNum * 2 < liveNum && liveNum > m_minNum)
        {
            // 访问了线程池,需要加锁
            lk.lock();
            // 一次性销毁两个
            m_exitNum = NUMBER;
            lk.unlock();
            // 让工作的线程自杀，无法做到直接杀死空闲线程，只能通知空闲线程让它自杀
            for (int i = 0; i < NUMBER; ++i)
                m_condQueue.notify_all(); // 工作线程阻塞在条件变量cond上
        }
    }
}

void ThreadPool::worker()
{
    // ThreadPool *pool = static_cast<ThreadPool *>(args);
    while (true)
    {
        unique_lock<mutex> lk(m_mutexPool);
        // 任务队列为空时的处理
        while (m_taskQ.empty() && !m_shutdown)
        {
            m_condQueue.wait(lk);

            // 判断是否要销毁线程
            if (m_exitNum > 0)
            {
                m_exitNum--;
                if (m_liveNum > m_minNum)
                {
                    m_liveNum--;
                    cout << "threadid: " << std::this_thread::get_id() << " exit......" << endl;
                    // 当前线程拥有互斥锁，所以需要解锁，不然会死锁
                    lk.unlock();
                    return;
                }
            }
        }
        // 判断线程池是否关闭了
        if (m_shutdown)
        {
            cout << "threadid: " << std::this_thread::get_id() << "exit......" << endl;
            return;
        }

        // 异常情况处理完了，开始执行任务
        Task task = m_taskQ.front();
        m_taskQ.pop();
        m_busyNum++;
        lk.unlock();

        // 取出Task任务后，就可以在当前线程中执行该任务了
        cout << "thread: " << std::this_thread::get_id() << " start working..." << endl;
        // task.m_func(task.m_args);
        task.m_func(task.m_args);

        // free(task.m_args);
        // task.m_args = nullptr;

        // 任务执行完毕,忙线程解锁
        cout << "thread: " << std::this_thread::get_id() << " end working..." << endl;
        lk.lock();
        m_busyNum--;
        lk.unlock();
    }
}