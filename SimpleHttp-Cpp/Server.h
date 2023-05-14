

class Server
{
public:
    Server(int port);

    void setListen();
    void run();

private:
    int sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length);
    int sendJson(int cfd);

private:
    int m_port;
    int m_lfd;
};