#include "EventLoop.h"
#include "WorkerThread.h"

class ThreadPool
{
public:
    ThreadPool(EventLoop *eventLoop, int count);
    void run();
    EventLoop *takeWorkerEventLoop();

private:
    bool m_isStart;
    EventLoop *m_mainLoop;
    // 子反应堆队列
    vector<WorkerThread *> m_workerThreads;
    int m_threadNum;
    int m_index;
};