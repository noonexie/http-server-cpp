#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "Server.h"
using namespace std;

class Server;
using callback = void (*)(void *);
// typedef void (Server::*callback)(void);

class Task
{
public:
    Task(callback func, void *args) : m_func(func), m_args(args) {}

public:
    void *m_args;
    callback m_func;
};

class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();
    void addTask(Task t);
    void addTask(callback f, void *a);

private:
    void manager();
    void worker();

private:
    bool m_shutdown;
    int m_minNum;
    int m_maxNum;

    // 管理线程要杀死的线程数
    int m_exitNum;
    // 正在运行的线程数
    int m_liveNum;
    // 正在执行任务的线程数
    int m_busyNum;
    thread m_managerId;
    vector<thread> m_workIds;
    queue<Task> m_taskQ;
    // 整个线程池的锁，获取线程池的信息
    mutex m_mutexPool;
    // 获取锁后，能不能执行还要看队列是否有任务这个条件
    condition_variable m_condQueue;
};
