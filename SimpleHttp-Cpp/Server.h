#include <arpa/inet.h>

struct SockInfo
{
    int fd;                  // 通信
    struct sockaddr_in addr; // 地址信息
};

class Server
{
public:
    Server(int port);
    void init();
    void run();

private:
    int sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length);
    int sendJson(int cfd);
    void recvHttpRequest(int cfd);
    void acceptClient();

private:
    int m_port;
    int m_lfd;
    int m_epfd;
    struct epoll_event *m_evs;
    const int m_maxNode = 128;
};