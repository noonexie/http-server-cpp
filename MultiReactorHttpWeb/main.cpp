#include <iostream>
#include "TcpServer.h"

int main(int argc, char *argv[])
{
    // 启动服务器
    TcpServer *server = new TcpServer(4);
    server->run();

    return 0;
}