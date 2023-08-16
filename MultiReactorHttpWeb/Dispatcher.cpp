#include "Dispatcher.h"

// 放在初始化列表效率高
Dispatcher::Dispatcher(EventLoop *evloop) : m_evLoop(evloop)
{
}

Dispatcher::~Dispatcher()
{
}

int Dispatcher::add()
{
    return 0;
}

int Dispatcher::remove()
{
    return 0;
}

int Dispatcher::modify()
{
    return 0;
}

int Dispatcher::dispatch(int timeout)
{
    return 0;
}
