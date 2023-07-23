#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

class EventLoop;
class TcpServer
{
public:
    TcpServer(int threadNum, int port = 10000);
    void setListen();
    void run();
    static int acceptConnection(void *arg);

private:
    int m_lfd;
    int m_port;
    int m_threadNum;
    ThreadPool *m_threadPool;
    EventLoop *m_mainLoop;
};