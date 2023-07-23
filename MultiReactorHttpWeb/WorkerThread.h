#include <string>

#include "EventLoop.h"
using namespace std;

class WorkerThread
{
public:
    WorkerThread(int index);
    void run();

    inline EventLoop *getEventLoop()
    {
        return m_eventLoop;
    }

private:
    void running();

private:
    EventLoop *m_eventLoop;
    thread *m_thread;
    string m_name;
};