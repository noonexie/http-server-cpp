#pragma once
#include <map>
#include <queue>
#include <thread>
#include <mutex>

#include "Channel.h"
#include "Dispatcher.h"
using namespace std;

class Channel;

enum ElemType : char
{
    ADD,
    REMOVE,
    MODIFY
};

struct ChannelElem
{
    Channel *channel;
    ElemType type;
};

class EventLoop
{
public:
    EventLoop();
    void run();
    void processTaskQ();
    void add();
    void remove();
    void modify();
    int addTask(Channel *channel, ElemType type);
    inline thread::id getThreadID()
    {
        return m_threadID;
    }

private:
    int m_fd;
    thread::id m_threadID;
    map<int, Channel> m_channalMap;
    queue<ChannelElem *> m_taskQ;
    mutex m_mutex;
    Dispatcher m_dispatcher;
};
