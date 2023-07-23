#include "Channel.h"

Channel::Channel(int fd, FDEvent event, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void *arg)
{
    m_fd = fd;
    m_event = event;
    m_readFunc = readFunc;
    m_writeFunc = writeFunc;
    m_destroyFunc = destroyFunc;
    m_arg = arg;
}
