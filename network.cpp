#include"client.hpp"
#include<stdexcept>

using namespace std;


// 构造函数定义
DictClient::DictClient(const string &ip,int port):sockfd_(-1),is_logged_in_(false)
{
    // 完成网络连接功能
    if(!connectToServer(ip,port))
    {
        throw runtime_error("connect to server error");
    }
}

DictClient::~DictClient()
{
    // 判断客户端是否存在
    if(sockfd_<=0)
    {
        doQuit();
        close(sockfd_);
    }
}


// 网络链接函数
bool DictClient::connectToServer(const string &ip,int port)
{
    // 创建通信的套接字文件描述符
    sockfd_=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd_<0)
    {
        perror("socket error");
        return false;
    }

    // 设置端口号快速重用
    int opt=1;
    if(setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0)
    {
        perror("setsockopt error");
        return false;
    }

    // 连接服务器
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_port=port;
    sin.sin_addr.s_addr=inet_addr(ip.c_str());

    // 链接服务器
    if(connect(sockfd_,(struct sockaddr*)&sin,sizeof(sin))<0)
    {
        perror("connect error");
        return false;
    }
    return true;
}