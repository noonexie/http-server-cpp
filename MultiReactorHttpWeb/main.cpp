#include "TcpServer.h"

int main(int argc, char const *argv[])
{
    TcpServer *server = new TcpServer(5);
    server->run();
    return 0;
}
