#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

class TcpServer
{
public:
    TcpServer(int threadNum, unsigned short port = 10000);
    // 初始化监听
    void setListen();
    // 启动服务器
    void run();
    static int acceptConnection(void *arg);

private:
    int m_threadNum;
    EventLoop *m_mainLoop;
    ThreadPool *m_threadPool;
    int m_lfd;
    unsigned short m_port;
};
