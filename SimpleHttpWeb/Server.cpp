#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <fcntl.h>
#include <errno.h>
#include <jsoncpp/json/json.h>
#include "Server.h"
#include "ThreadPool.h"

using namespace std;
using namespace Json;

Server::Server(int port = 10000) : m_port(port)
{
    int ret;
    // 1.创建套接字
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd == -1)
    {
        perror("socket");
        exit(0);
    }
    // 2.设置端口复用
    int opt = 1;
    ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        exit(0);
    }
    // 2.设置非阻塞
    // int flag = fcntl(m_lfd, F_GETFL);
    // flag |= O_NONBLOCK;
    // fcntl(m_lfd, F_SETFL, flag);
    // 3.绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }
    // 4.启动监听
    ret = listen(m_lfd, 128);
    if (ret == -1)
    {
        perror("listen");
        exit(0);
    }
    // 5.创建epoll模型
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
    {
        perror("epoll_create");
        exit(0);
    }
    // 6.m_lfd上树
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_lfd;
    ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_lfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    // 设置线程池最小5个线程，最大10个线程
    m_threadPool = new ThreadPool(5, 10);
    if (m_threadPool == nullptr)
    {
        perror("ThreadPool");
        exit(0);
    }
    // 数据库连接
    m_mysql = mysql_init(NULL);
    if (m_mysql == NULL)
    {
        printf("mysql_init() error\n");
        exit(0);
    }
    // 连接数据库服务器
    m_mysql = mysql_real_connect(m_mysql, "localhost", "root", "789456123",
                                 "skin", 3306, NULL, 0);
    if (m_mysql == NULL)
    {
        printf("mysql_real_connect() error\n");
        exit(0);
    }
    printf("mysql api使用的默认编码: %s\n", mysql_character_set_name(m_mysql));
    // 设置编码为utf8
    mysql_set_character_set(m_mysql, "utf8");
    printf("mysql api使用的修改之后的编码: %s\n", mysql_character_set_name(m_mysql));
    printf("恭喜, 连接数据库服务器成功了...\n");
}

void Server::run()
{
    cout << "服务器已启动，端口：" << m_port
         << " m_lfd: " << m_lfd << endl;

    struct epoll_event evs[1024];
    // 1024改了下面size不需要修改
    int size = sizeof(evs) / sizeof(struct epoll_event);

    while (1)
    {
        int num = epoll_wait(m_epfd, evs, size, -1);
        for (int i = 0; i < num; i++)
        {
            // 取出当前的文件描述符
            int curfd = evs[i].data.fd;
            // 判断是监听文件描述符还是数据交互文件描述符
            if (curfd == m_lfd)
            {
                // 与客户端建立连接
                // acceptClient(this);

                // 多线程模式
                // thread thread_accept(&Server::acceptClient, this);
                // cout << "建立连接线程id: " << thread_accept.get_id() << endl;
                // thread_accept.detach();

                // 线程池模式
                m_threadPool->addTask(acceptClient, this);
            }
            else
            {
                // 多线程进行数据交互
                // m_cfd = curfd;
                thread thread_recv(&Server::recvHttpRequest, this, curfd);
                cout << "交互线程id: " << thread_recv.get_id() << endl;
                thread_recv.detach();

                // 线程池模式
                // m_cfd = curfd;
                // m_threadPool->addTask(recvHttpRequest, this);
            }
        }
    }
}

void Server::acceptClient(void *arg)
{
    Server *server = static_cast<Server *>(arg);
    // 1. 建立新的连接
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    int cfd = accept(server->m_lfd, (struct sockaddr *)&addr, &len);
    if (cfd <= 0)
    {
        perror("accept");
        // return;
        exit(0);
    }
    cout << "有新连接建立, cfd: " << cfd
         << " 客户端IP: " << inet_ntoa(*((struct in_addr *)&addr.sin_addr))
         << " 客户端PORT: " << addr.sin_port << endl;
    // 2. 设置非阻塞：效率高(设置用于通信的文件描述符在epll中为边沿非阻塞工作模式)
    // 边沿模式：检测到读事件后只会通知一次，所以接收数据测要循环接受直至全部数据接收完毕
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);
    // 3. 添加到epoll树中进行监听
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = cfd;
    int ret = epoll_ctl(server->m_epfd, EPOLL_CTL_ADD, cfd, &ev);
    cout << "已添加新文件描述符至epfd: " << cfd << endl;
    if (ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }
}

// 数据交互
void Server::recvHttpRequest(int cfd)
{
    // Server *server = static_cast<Server *>(arg);
    cout << "交互描述符：" << cfd << endl;

    int len, totle = 0;
    char tmp[1024] = {0};
    char buf[4096] = {0};
    // 这个套接字是非阻塞的，数据读完了还是会recv并且返回-1
    // 如果是阻塞的就会阻塞在recv之前
    while ((len = recv(cfd, tmp, sizeof tmp, 0)) > 0)
    {
        cout << "开始接收数据..." << endl;
        if (totle + len < sizeof buf)
        {
            memcpy(buf + totle, tmp, len);
        }
        totle += len;
    }

    // 判断数据是否接收完毕
    if (len == -1 && errno == EAGAIN)
    {
        // 表示数据已接收完毕，errno非异常值
        // 开始处理业务逻辑
        cout << "数据已接收完毕: " << buf << endl;
        string ans = parseHttp(buf);
        sendHttp(cfd, ans);
        cout << "send OK" << endl;
    }
    else if (len == 0)
    {
        // 客户端断开了连接
        cout << "客户端断开了连接" << endl;
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, cfd, NULL);
        close(cfd);
    }
    else
    {
        printf("数据接收失败\n");
        perror("recv");
        exit(0);
    }
}

string Server::parseHttp(char *buf)
{
    string http = buf;
    vector<string> parse(2);
    int i = 0, j = 0, k = 0;
    while (k < 2)
    {
        if (http[j] == ' ')
        {
            parse[k] = http.substr(i, j - i);
            i = j + 1;
            k++;
        }
        j++;
    }

    if (parse[0] == "GET" && parse[1].find("/user/") != -1)
    {
        for (int i = parse[1].size() - 1; i >= 0; --i)
        {
            if (parse[1][i] == '/')
            {
                string userId = parse[1].substr(i + 1, parse[1].size() - i);
                string strSQL = "select * from user where id = " + userId;
                int ret = mysql_query(m_mysql, strSQL.c_str());
                if (ret != 0)
                {
                    printf("mysql_query() a失败了, 原因: %s\n", mysql_error(m_mysql));
                    break;
                }
                // 取出结果集
                MYSQL_RES *res = mysql_store_result(m_mysql);
                if (res == NULL)
                {
                    printf("mysql_store_result() 失败了, 原因: %s\n", mysql_error(m_mysql));
                    break;
                }
                // 得到结果集中的列数
                int num = mysql_num_fields(res);
                // 得到所有列的名字, 并且输出
                MYSQL_FIELD *fields = mysql_fetch_fields(res);
                // for (int i = 0; i < num; ++i)
                // {
                //     printf("%s\t\t", fields[i].name);
                // }
                // printf("\n");
                // 遍历结果集中所有的行
                MYSQL_ROW row;
                Value root;
                root["code"] = "0";
                root["msg"] = "成功";
                Value data;
                while ((row = mysql_fetch_row(res)) != NULL)
                {
                    // 将当前行中的每一列信息读出
                    for (int i = 0; i < num; ++i)
                    {
                        // printf("%s\t\t", row[i]);
                        if (row[i])
                            data[fields[i].name] = row[i];
                        else
                            data[fields[i].name] = "";
                    }
                    // printf("\n");
                }
                // 释放资源 - 结果集
                mysql_free_result(res);
                root["data"] = data;
                StyledWriter Writer_style;
                return Writer_style.write(root);
            }
        }
    }
    return "";
}

void Server::sendHttp(int cfd, string ans)
{
    // 状态行
    char buf[4096] = {0};
    sprintf(buf, "HTTP/1.1 %d %s\r\n", 200, "OK");
    // 响应头
    sprintf(buf + strlen(buf), "Content-Type: %s\r\n", "application/json");
    sprintf(buf + strlen(buf), "Content-Length: %d\r\n\r\n", ans.size());
    // sprintf(buf + strlen(buf), content);
    // cout << buf << endl;
    send(cfd, buf, strlen(buf), 0);
    // 数据
    // cout << buf << endl;
    send(cfd, ans.c_str(), ans.size(), 0);
}
