#ifndef CLIENT_H
#define CLIENT_H


#include"message.hpp"

class DictClient
{
    public:
    DictClient(const string &ip,int port);           //构造函数
    ~DictClient();
    void run();

    private:
    int sockfd_;                                    //客户端套接字文件描述符
    string username_;                               //用户名
    bool is_logged_in_;                             //判断是否已经登录

    // 网络连接函数
    bool connectToServer(const string &ip,int port);


    // 业务功能函数
    void showMainMenu();                            //展示主菜单
    void showUserMenu();                            //展示业务菜单
    bool doRegister();                              //注册功能
    bool doLogin();                                 //登入功能
    void doQuerry();
    void doHistory();
    void doQuit();
};

#endif