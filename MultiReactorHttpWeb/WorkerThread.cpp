#include "WorkerThread.h"

WorkerThread::WorkerThread(int index)
{
    m_eventLoop = nullptr;
    m_name = "SubThread-" + to_string(index);
}

// 启动线程执行函数
void WorkerThread::run()
{
    m_thread = new thread(&WorkerThread::running, this);
}

// 让反应堆启动
void WorkerThread::running()
{
    m_eventLoop->run();
}
