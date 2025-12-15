#include"server.hpp"            //服务器头文件
#include"message.hpp"           //消息协议头文件
#include<bits/stdc++.h>
#include<time.h>                //有关时间的头文件
#include<thread>                //C++线程支持库 



/*
服务器构造函数：完成套接字的创建，端口号快速重用，绑定ip和端口号
*/

Server::Server(shared_ptr<DatabaseManger> db_manager,const string ip,int port)
:db_manager_(db_manager),ip_(ip),port_(port),running_(false)
{
    // 创建套接字文件描述符
    sfd_=socket(AF_INET,SOCK_STREAM,0);
    if(sfd_<0)
    {
        throw runtime_error("socket error");            //抛出异常
    }

    // 设置端口号快速重用
    int opt=1;
    if(setsockopt(sfd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0)
    {
        close(sfd_);
        throw runtime_error("setsocketopen error");
    }

    // 绑定ip和端口号
    sockaddr_in sin;
    sin.sin_family=AF_INET;                                 //通信域绑定
    sin.sin_port=htons(port);                               //端口号绑定
    sin.sin_addr.s_addr=inet_addr(ip.c_str());              //ip地址

    if(bind(sfd_,(sockaddr *)&sin,sizeof(sin))<0)
    {
        close(sfd_);
        throw runtime_error("bind error");
    }
}

/*
析构函数关闭服务器
*/

Server::~Server()
{
    stop();
}


/*
启动服务器
*/


bool Server::start()
{
    //启动监听
    if(listen(sfd_,5)<0)
    {
        cerr<<"listen error"<<endl;
        return false;
    }

    // 设置运行状态
    running_=true;
    cout<<"Server started:"<<ip_<<" :"<<port_<<endl;


    // 等待客户端发送链接请求：循环完成
    while(running_)
    {
        sockaddr_in client_addr;                    //接受客户端地址信息结构体
        socklen_t addr_len=sizeof(client_addr);     //接受客户端地址信息结构体大小


        // 接受客户端连接请求操作
        int cfd=accept(sfd_, (struct sockaddr*)&client_addr,&addr_len);

        if(cfd<0)
        {
            if(running_)
            {
                cerr<<"accept error"<<endl;
            }
            continue;
        }

        // 创建分支线程，在分支线程中，进行跟客户端的通信
        thread([this,cfd,client_addr](){
            cout<<"Client connected:"<<inet_ntoa(client_addr.sin_addr)<<":"<<ntohs(client_addr.sin_port)<<endl;

            // 调用客户端处理函数
            handleClient(cfd,client_addr);

        }).detach();            //将线程设置为分离态，资源由系统回收

    }
        
    return true;

}

/*
停止服务器函数
//关闭套接字文字描述符
*/

bool Server::stop()
{
    if(running_)
    {
        running_=false;
        close(sfd_);
        cout<<"Server stopped"<<endl;
        return true;
    }
    return false;
}



/*
    处理客户端请求，分为不同的请求模式，接收客户端消息，根据不同类型进行不同的处理
    参数1：客户端套接字文件描述符
    参数2：客户端地址信息结构体
*/

void Server::handleClient(int cfd,sockaddr_in client_addr)
{
    Msg msg;        //用于储存收发的数据

    while(true)
    {
        //接受客户发送的信息
        ssize_t recv_len=recv(cfd,&msg,sizeof(msg),0);

        if(recv_len<=0)
        {
            if(recv_len==0) //当前客户端已下线
            {
                cout<<"Client disconnect:"<<inet_ntoa(client_addr.sin_addr)<<endl;
            }else{
                cout<<"recv error"<<endl;
            }
            break;
        }

        // 程序执行至此说明，已经读到客户端发来的消息
        // 转换为字节序
        msg.hostByteOrder();

        // 准备响应消息
        Msg respone;                //用于回复消息的容器

        //表示要给谁发送消息
        strncpy(respone.name,msg.name,sizeof(respone.name));

        // 回复消息的类型
        respone.type=msg.type;

        // 根据不同的消息请求处理不同的响应
        switch (msg.type)
        {
        case R:
            // 执行注册操作
            int success=db_manager_->registerUser(msg.name,msg.text);
            strcpy(respone.text,success?"**OK**" : "**EXISTS**");//如果成功注册回复OK，否则回复EXISTS
            break;
        
        default:
            break;
        }
    }
}