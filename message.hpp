#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include<bits/stdc++.h>
#include<sqlite3.h>
#include<mutex>
#include<memory>    //智能指针头文件
#include<string> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>   // sockaddr_in 在这里
#include <arpa/inet.h>    // inet_addr、inet_ntoa 等函数
#include <unistd.h>       // close()
#include <thread>             //线程头文件
using namespace std;

// 定义操作类型
#define R 1             //用户注册
#define L 2             //用户登入
#define Q 3             //用户退出
#define S 4             //单词查询
#define H 5             //历史记录

// 定义客户端与服务端进行数据通信的协议
struct Msg
{
    int type;       //操作类型
    char name[20];  //用户名称
    char text[128]; //文本内容      单词或者密码

    // 定义相关字序的转化函数
    // 主机字节序转网络字节序
    void networkByteOrder()
    {
        type=htonl(type);
    }

    // 网络字节序转主机字节序
    void hostByteOrder()
    {
        type=ntohl(type);
    }

    
};





#endif 