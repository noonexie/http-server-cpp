#include <sys/socket.h>
#include <cstdio>
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>

#include "TcpServer.h"

TcpServer::TcpServer(int threadNum, int port) : m_threadNum(threadNum), m_port(port)
{
    m_mainLoop = new EventLoop();
    m_threadPool = new ThreadPool(m_mainLoop, threadNum);
    setListen();
}

void TcpServer::setListen()
{
    // 创建socket
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd == -1)
    {
        perror("socket");
        exit(0);
    }
    // 设置端口复用
    /********************************************************
     * 描述: 缺省条件下，一个套接口不能与一个已在使用中的本地地址捆绑（参见bind()）。
     * 但有时会需要“重用”地址。因为每一个连接都由本地地址和远端地址的组合唯一确定，
     * 所以只要远端地址不同，两个套接口与一个地址捆绑并无大碍。
     * 为了通知套接口实现不要因为一个地址已被一个套接口使用就不让它与另一个套接口捆绑，
     * 应用程序可在bind()调用前先设置SO_REUSEADDR选项。请注意仅在bind()调用时该选项才被解释；
     * 故此无需（但也无害）将一个不会共用地址的套接口设置该选项，或者在bind()对这个或其他套接口无影响情况下设置或清除这一选项。
     ********************************************************/
    int opt = 1;
    int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        exit(0);
    }
    // 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_lfd, (struct sockaddr *)&addr, sizeof addr);
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }
    // 启动监听
    ret = listen(m_lfd, 128);
    if (-1 == ret)
    {
        perror("listen");
        exit(0);
    }
}

void TcpServer::run()
{
    cout << "服务器开始启动。。。" << endl;

    // 启动线程池:启动子反应堆
    m_threadPool->run();

    // 主channel加入检测
    Channel *channel = new Channel(m_lfd, FDEvent::ReadEvent, acceptConnection, nullptr, nullptr, this);
    m_mainLoop->addTask(channel, ElemType::ADD);
}

// socket有事件发生，回调函数内处理事件
int TcpServer::acceptConnection(void *arg)
{
    TcpServer *server = static_cast<TcpServer *>(arg);

    // 和客户建立连续
    int cfd = accept(server->m_lfd, NULL, NULL);
    // 取一个子反应堆承接
    EventLoop *evLoop = server->m_threadPool->takeWorkerEventLoop();
}