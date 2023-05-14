#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "Server.h"

using namespace std;

Server::Server(int port = 10000) : m_port(port)
{
}

void Server::setListen()
{
    // 1.创建套接字
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd < 0)
    {
        perror("socket");
        return;
    }
    // 2.绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(m_lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        perror("bind");
        return;
    }
    // 3.启动监听
    ret = listen(m_lfd, 128);
    if (ret < 0)
    {
        perror("listen");
        return;
    }
}

void Server::run()
{
    int cfd;
    struct sockaddr_in clientAddr;
    socklen_t clientAddr_len = sizeof(clientAddr);
    cout << "服务器已启动，端口：" << m_port << endl;
    while (1)
    {
        cfd = accept(m_lfd, (sockaddr *)&clientAddr, &clientAddr_len);
        // 数据交换
        int n, i = 0;
        char buf[1024] = {0};
        while (1)
        {
            n = read(cfd, buf, sizeof(buf));
            if (n == 0) // 有客户端断开连接
            {
                printf("有客户端断开连接\n");
                break;
            }
            if (n < 0)
            {
                printf("aaaaaaaa\n");
                break;
            }
            // inet_ntop(AF_INET,&clientAddr.sin_addr,str,sizeof(str));
            // ntohs(clientAddr.sin_port);
            printf("已收到第%d次数据:%s\n", i++, buf);
            // sleep(1);
            sendHeadMsg(cfd, 200, "OK", "application/json", -1);
            sendJson(cfd);
            cout << "send OK" << endl;
            break;
        }
        close(cfd);
    }
}

int Server::sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length)
{
    // 状态行
    char buf[4096] = {0};
    char content[] = "{\"agentIsMobile\":false,\"platform\":\"windows\",\"version\":null}";
    sprintf(buf, "HTTP/1.1 %d %s\r\n", status, descr);
    // 响应头
    sprintf(buf + strlen(buf), "Content-Type: %s\r\n", type);
    sprintf(buf + strlen(buf), "Content-Length: %d\r\n\r\n", strlen(content));
    // sprintf(buf + strlen(buf), content);
    // cout << buf << endl;
    send(cfd, buf, strlen(buf), 0);
    return 0;
}

int Server::sendJson(int cfd)
{
    char buf[] = "{\"agentIsMobile\":false,\"platform\":\"windows\",\"version\":null}";
    cout << buf << endl;
    send(cfd, buf, strlen(buf), 0);
}