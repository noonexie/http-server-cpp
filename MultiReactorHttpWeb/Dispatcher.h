
class EventLoop;

class Dispatcher
{
public:
    Dispatcher(EventLoop *eventLoop);
    virtual ~Dispatcher();
    // 添加
    virtual int add();
    // 删除
    virtual int remove();
    // 修改
    virtual int modify();
};