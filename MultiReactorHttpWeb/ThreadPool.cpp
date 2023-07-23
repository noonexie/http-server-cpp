#include <cassert>

#include "WorkerThread.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(EventLoop *ventLoop, int count)
{
}

void ThreadPool::run()
{
    assert(!m_isStart); // 传入为假则程序终止
    if (m_mainLoop->getThreadID() != this_thread::get_id())
        exit(0);
    m_isStart = true;
    // 启动所有子反应堆
    if (m_threadNum > 0)
    {
        for (int i = 0; i < m_threadNum; ++i)
        {
            WorkerThread *subThread = new WorkerThread(i);
            subThread->run();
            m_workerThreads.push_back(subThread);
        }
    }
}

EventLoop *ThreadPool::takeWorkerEventLoop()
{
    assert(m_isStart);
    if (m_mainLoop->getThreadID() != this_thread::get_id())
        exit(0);
    EventLoop *evLoop = m_mainLoop;
    if (m_threadNum > 0)
    {
        evLoop = m_workerThreads[m_index]->getEventLoop();
        m_index = ++m_index % m_threadNum;
    }
    return evLoop;
}
