#include "EventLoop.h"

EventLoop::EventLoop() : m_dispatcher(this)
{
    m_fd = 0;
    m_channalMap.clear();
}

void EventLoop::run()
{
}

void EventLoop::processTaskQ()
{
    while (!m_taskQ.empty())
    {
        ChannelElem *elem = m_taskQ.front();
        m_taskQ.pop();
        if (elem->type == ElemType::ADD)
        {
        }
    }
}

void EventLoop::add()
{
}

void EventLoop::remove()
{
}

void EventLoop::modify()
{
}

int EventLoop::addTask(Channel *channel, ElemType type)
{
    // 主线程会往子线程的任务队列添加任务，所以要加锁
    m_mutex.lock();
    ChannelElem *elem = new ChannelElem;
    elem->channel = channel;
    elem->type = type;
    m_taskQ.push(elem);
    m_mutex.unlock();
    // 处理节点
    /*
     * 细节:
     *   1. 对于链表节点的添加: 可能是当前线程也可能是其他线程(主线程)
     *       1). 修改fd的事件, 当前子线程发起, 当前子线程处理
     *       2). 添加新的fd, 添加任务节点的操作是由主线程发起的
     *   2. 不能让主线程处理任务队列, 需要由当前的子线程取处理
     */
    if (m_threadID == this_thread::get_id())
    {
        // 当前子线程处理任务
    }
}
