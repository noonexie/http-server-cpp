#pragma once
#include <functional>

// 定义文件描述符的读写事件
enum class FDEvent
{
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};

using handleFunc = std::function<int(void *)>;

class Channel
{
public:
    Channel(int fd, FDEvent event, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void *arg);
    // Channel();
    handleFunc m_readFunc;
    handleFunc m_writeFunc;
    handleFunc m_destroyFunc;

private:
    int m_fd;
    FDEvent m_event;
    void *m_arg;
};