#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>

struct ThreadPool *threadPoolInit(struct EventLoop *mainLoop, int count)
{
    struct ThreadPool *pool = (struct ThreadPool *)malloc(sizeof(struct ThreadPool));
    pool->index = 0;
    pool->isStart = false;
    pool->mainLoop = mainLoop;
    pool->threadNum = count;
    pool->workerThreads = (struct WorkerThread *)malloc(sizeof(struct WorkerThread) * count);
    return pool;
}

void threadPoolRun(struct ThreadPool *pool)
{
    assert(pool && !pool->isStart);
    if (pool->mainLoop->threadID != pthread_self()) // 启动线程池的肯定是主线程才对
    {
        exit(0);
    }
    pool->isStart = true;
    if (pool->threadNum)
    {
        for (int i = 0; i < pool->threadNum; ++i)
        {
            workerThreadInit(&pool->workerThreads[i], i);
            workerThreadRun(&pool->workerThreads[i]);
        }
    }
}

// 主线程挑一个子线程，再从子线程中取出反应堆实例
struct EventLoop *takeWorkerEventLoop(struct ThreadPool *pool)
{
    assert(pool->isStart);
    if (pool->mainLoop->threadID != pthread_self())
    {
        exit(0);
    }
    // 从线程池中找一个子线程, 然后取出里边的反应堆实例
    struct EventLoop *evLoop = pool->mainLoop;
    if (pool->threadNum > 0) // 如果没有子线程，返回主线程反应堆实例
    {
        evLoop = pool->workerThreads[pool->index].evLoop;
        pool->index = ++pool->index % pool->threadNum;
    }
    return evLoop;
}
