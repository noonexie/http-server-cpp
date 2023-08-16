// #define _GNU_SOURCE
#include <cassert>
#include <cstring>
#include <unistd.h>

#include "HttpRequest.h"
#include "TcpConnection.h"

char *HttpRequest::splitRequestLine(const char *start, const char *end, const char *sub, function<void(string)> callback)
{
    char *space = const_cast<char *>(end);
    if (sub != nullptr)
    {
        space = static_cast<char *>(memmem(start, end - start, sub, strlen(sub)));
        assert(space != nullptr);
    }
    int length = space - start;
    callback(string(start, length));
    return space + 1;
}

// 将字符转换为整形数
int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

HttpRequest::HttpRequest()
{
    reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
    m_curState = PrecessState::ParseReqLine;
    m_method = m_url = m_version = string(); // ""
    m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty())
    {
        return;
    }
    m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
    auto item = m_reqHeaders.find(key);
    if (item == m_reqHeaders.end())
    {
        return string();
    }
    return item->second;
}

bool HttpRequest::parseRequestLine(Buffer *readBuf)
{
    // 读出请求行, 保存字符串结束地址
    char *end = readBuf->findCRLF();
    // 保存字符串起始地址
    char *start = readBuf->data();
    // 请求行总长度
    int lineSize = end - start;

    if (lineSize > 0)
    {
        auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", methodFunc);
        auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", urlFunc);
        auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
        splitRequestLine(start, end, nullptr, versionFunc);
        // 为解析请求头做准备
        readBuf->readPosIncrease(lineSize + 2);
        // 修改状态
        setState(PrecessState::ParseReqHeaders);
        return true;
    }
    return false;
}

bool HttpRequest::parseRequestHeader(Buffer *readBuf)
{
    char *end = readBuf->findCRLF();
    if (end != nullptr)
    {
        char *start = readBuf->data();
        int lineSize = end - start;
        // 基于: 搜索字符串
        char *middle = static_cast<char *>(memmem(start, lineSize, ": ", 2));
        if (middle != nullptr)
        {
            int keyLen = middle - start;
            int valueLen = end - middle - 2;
            if (keyLen > 0 && valueLen > 0)
            {
                string key(start, keyLen);
                string value(middle + 2, valueLen);
                addHeader(key, value);
            }
            // 移动读数据的位置
            readBuf->readPosIncrease(lineSize + 2);
        }
        else
        {
            // 请求头被解析完了, 跳过空行
            readBuf->readPosIncrease(2);
            // 修改解析状态
            // 忽略 post 请求, 按照 get 请求处理
            setState(PrecessState::ParseReqDone);
        }
        return true;
    }
    return false;
}

bool HttpRequest::parseHttpRequest(Buffer *readBuf, HttpResponse *response, Buffer *sendBuf, int socket)
{
    bool flag = true;
    while (m_curState != PrecessState::ParseReqDone)
    {
        switch (m_curState)
        {
        case PrecessState::ParseReqLine:
            flag = parseRequestLine(readBuf);
            break;
        case PrecessState::ParseReqHeaders:
            flag = parseRequestHeader(readBuf);
            break;
        case PrecessState::ParseReqBody:
            break;
        default:
            break;
        }
        if (!flag)
        {
            return flag;
        }
        // 判断是否解析完毕了, 如果完毕了, 需要准备回复的数据
        if (m_curState == PrecessState::ParseReqDone)
        {
            // 1. 根据解析出的原始数据, 对客户端的请求做出处理
            processHttpRequest(response);
            // 2. 组织响应数据并发送给客户端
            response->prepareMsg(sendBuf, socket);
        }
    }
    m_curState = PrecessState::ParseReqLine; // 状态还原, 保证还能继续处理第二条及以后的请求
    return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse *response)
{
    if (strcasecmp(m_method.data(), "get") != 0)
    {
        return -1;
    }
    m_url = decodeMsg(m_url);
    // 处理客户端请求的静态资源(目录或者文件)
    string json = "";
    if (m_url.find("/user/") != -1)
    {
        json = response->userJson(m_url);
    }

    response->setFileName(json);
    response->setStatusCode(StatusCode::OK);
    response->addHeader("Content-type", "application/json");
    response->addHeader("Content-length", to_string(json.size()));
    response->sendDataFunc = sendJson;

    return false;
}

string HttpRequest::decodeMsg(string msg)
{
    string str = string();
    const char *from = msg.data();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }
    }
    str.append(1, '\0');
    return str;
}

void HttpRequest::sendJson(string json, Buffer *sendBuf, int cfd)
{
    if (json.size() > 0)
    {
        // send(cfd, buf, len, 0);
        sendBuf->appendString(json.c_str(), json.size());
        sendBuf->sendData(cfd);
    }
}
