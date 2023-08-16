#include <jsoncpp/json/json.h>
#include "HttpResponse.h"
#include "Log.h"

using namespace Json;

HttpResponse::HttpResponse()
{
    m_statusCode = StatusCode::Unknown;
    m_headers.clear();
    m_fileName = string();
    sendDataFunc = nullptr;

    // 数据库连接
    m_mysql = mysql_init(NULL);
    if (m_mysql == NULL)
    {
        Debug("mysql_init() error");
        exit(0);
    }
    // 连接数据库服务器
    m_mysql = mysql_real_connect(m_mysql, "localhost", "root", "789456123",
                                 "skin", 3306, NULL, 0);
    if (m_mysql == NULL)
    {
        Debug("mysql_real_connect() error");
        exit(0);
    }
    Debug("mysql api使用的默认编码: %s", mysql_character_set_name(m_mysql));
    // 设置编码为utf8
    mysql_set_character_set(m_mysql, "utf8");
    Debug("mysql api使用的修改之后的编码: %s", mysql_character_set_name(m_mysql));
    Debug("恭喜, 连接数据库服务器成功了...");
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty())
    {
        return;
    }
    m_headers.insert(make_pair(key, value));
}

void HttpResponse::prepareMsg(Buffer *sendBuf, int socket)
{
    // 状态行
    char tmp[1024] = {0};
    int code = static_cast<int>(m_statusCode);
    sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, m_info.at(code).data());
    sendBuf->appendString(tmp);
    // 响应头
    for (auto it = m_headers.begin(); it != m_headers.end(); ++it)
    {
        sprintf(tmp, "%s: %s\r\n", it->first.data(), it->second.data());
        sendBuf->appendString(tmp);
    }
    // 空行
    sendBuf->appendString("\r\n");
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(socket);
#endif

    // 回复的数据
    sendDataFunc(m_fileName, sendBuf, socket);
}

string HttpResponse::userJson(string url)
{
    for (int i = url.size() - 1; i >= 0; --i)
    {
        if (url[i] == '/')
        {
            string userId = url.substr(i + 1, url.size() - i);
            string strSQL = "select * from user where id = " + userId;
            int ret = mysql_query(m_mysql, strSQL.c_str());
            if (ret != 0)
            {
                Debug("mysql_query() a失败了, 原因: %s", mysql_error(m_mysql));
                break;
            }
            // 取出结果集
            MYSQL_RES *res = mysql_store_result(m_mysql);
            if (res == NULL)
            {
                Debug("mysql_store_result() 失败了, 原因: %s", mysql_error(m_mysql));
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
    return "";
}