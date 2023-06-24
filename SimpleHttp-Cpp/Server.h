#pragma once
#include <arpa/inet.h>
#include "ThreadPool.h"

class ThreadPool;

struct SockInfo
{
    int fd;                  // 通信
    struct sockaddr_in addr; // 地址信息
};

class RecvInfo
{
public:
    RecvInfo(int cfd, int epfd) : m_cfd(cfd), m_epfd(epfd)
    {
    }

public:
    int m_cfd;
    int m_epfd;
};

class Server
{
public:
    Server(int port);
    // void init();
    void run();

private:
    int sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length);
    int sendJson(int cfd);
    // static void recvHttpRequest(void *arg);
    void recvHttpRequest(int cfd);
    static void acceptClient(void *arg);

private:
    // int m_cfd;
    int m_port;
    int m_lfd;
    int m_epfd;
    struct epoll_event *m_evs;
    const int m_maxNode = 128;
    ThreadPool *m_threadPool;
};