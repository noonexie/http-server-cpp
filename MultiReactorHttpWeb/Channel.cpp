#include "Channel.h"

Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void *arg)
{
    m_arg = arg;
    m_fd = fd;
    m_events = (int)events;
    readCallback = readFunc;
    writeCallback = writeFunc;
    destroyCallback = destroyFunc;
}

void Channel::writeEventEnable(bool flag)
{
    if (flag)
    {
        // 添加写事件
        // m_events |= (int)FDEvent::WriteEvent;
        m_events |= static_cast<int>(FDEvent::WriteEvent);
    }
    else
    {
        // 去掉写事件
        m_events = m_events & ~(int)FDEvent::WriteEvent;
    }
}

bool Channel::isWriteEventEnable()
{
    return m_events & (int)FDEvent::WriteEvent;
}
