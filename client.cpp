#include "client.hpp"
#include <bits/stdc++.h>
#include <vector>
#include <sqlite3.h>
#include <mutex>
#include <memory> //智能指针头文件
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in 在这里
#include <arpa/inet.h>  // inet_addr、inet_ntoa 等函数
#include <unistd.h>     // close()
#include <thread>       //线程头文件
using namespace std;

/*主程序循环*/
void DictClient::run()
{
    while (true)
    {
        showMainMenu(); // 显示主菜单
        int choice;     // 选择功能
        cin >> choice;
        cin.ignore(); // 清除缓存区

        switch (choice)
        {
        case R:
            doRegister(); //  调用登入功能
            break;
        case L:
            if (doLogin()) // 调用登录功能
            {
                showUserMenu(); // 调用用户界面
            }
            break;
        case Q:
            doQuit(); // 调用退出界面
            break;
        default:
            cout << "无效的选择" << endl;
            break;
        }
        cout << "按回车继续。。。";
        cin.get(); // 吸收回车
    }
}

void DictClient::showMainMenu()
{
    system("clear");
    cout << "***************************" << endl;
    cout << "*********1、注册***********" << endl;
    cout << "*********2、登录***********" << endl;
    cout << "*********3、退出***********" << endl;
    cout << "***************************" << endl;
    cout << "请选择：";
}

void DictClient::showUserMenu()
{
    while (is_logged_in_)
    {
        system("clear"); // 清屏
        cout << "当前用户：" << username_ << endl;
        cout << "*********************************" << endl;
        cout << "**********1、查单词***************" << endl;
        cout << "**********2、历史记录***************" << endl;
        cout << "**********3、返回上一级***************" << endl;
        cout << "*********************************" << endl;
        cout << "请选择：";
        int choice;
        cin >> choice;
        cin.ignore();

        // 对选择内容进行分支
        switch (choice)
        {
        case 1:
            doQuerry();
            break;
        case 2:
            doHistory();
            break;
        case 3:
            doQuit();
            break;

        default:
            cout << "无效的选择" << endl;
            break;
        }
        cout << "按回车继续...." << endl;
        cin.get();
    }
}

// 用户注册函数
bool DictClient::doRegister()
{
    // 定义相关数据信息
    Msg msg;
    msg.type = R; // 表示注册类型

    // 提示并输入用户名和密码
    cout << "请输入用户名称(最长19个字符):";
    cin.getline(msg.name, sizeof(msg.name));
    cout << "请输入密码(最长127个字符):";
    cin.getline(msg.text, sizeof(msg.text));

    // 将上面的信息转化成网络字节序
    msg.networkByteOrder();
    // 发送个服务器
    if (send(sockfd_, &msg, sizeof(msg), 0) < 0)
    {
        perror("注册失败");
        return false;
    }

    // 到此说明已经发送成功
    if (recv(sockfd_, &msg, sizeof(msg), 0) <= 0)
    {
        perror("接受注册信息响应失败");
        return false;
    }

    // 到此说明已经接受到发送的消息
    // 将网络字节序转化成本机字节序
    msg.hostByteOrder();

    // 处理服务器给出的响应
    if (strcmp(msg.text, "**OK**") == 0)
    {
        cout << "注册功能" << endl;
        return true;
    }
    else if (strcmp(msg.text, "**EXISTS**") == 0)
    {
        cout << "注册失败，用户名已经存在" << endl;
    }
    else
    {
        cout << "注册失败，错误未知" << endl;
    }
    return false;
}

bool DictClient::doLogin()
{
    // 定义相关数据信息
    Msg msg;
    msg.type = L;

    cout << "请输入用户姓名";
    cin.getline(msg.name, sizeof(msg.name));
    cout << "请输入密码：";
    cin.getline(msg.text, sizeof(msg.text));

    // 转化成网络字节序
    msg.networkByteOrder();
    // 发送给服务器
    if (send(sockfd_, msg.text, sizeof(msg.text), 0) < 0)
    {
        perror("发送登入功能失败");
        return false;
    }

    if (recv(sockfd_, &msg, sizeof(msg), 0) <= 0)
    {
        perror("接受登录响应失败");
        return false;
    }

    // 接受成功转化成本机字节序
    msg.hostByteOrder();

    // 处理接受的消息
    if (strcmp(msg.text, "**OK**") == 0)
    {
        cout << "登入成功" << endl;
        username_ = msg.name;
        is_logged_in_ = 1; // 更新状态
        return true;
    }
    else if (strcmp(msg.text, "**EXISTS**") == 0)
    {
        cout << "登入失败，用户已在线" << endl;
    }
    else
    {
        cout << "未知错误" << endl;
    }
    return false;
}

// 查询单词功能
void DictClient::doQuerry()
{
    Msg msg; // 创建传输体
    // 标记状态
    msg.type = S;

    strcpy(msg.name, username_.c_str()); // 复制查询的单词名
    while (true)
    {
        cout << "请输入要查询的单词(输入#后结束查询):";
        cin.getline(msg.text, sizeof(msg.text));

        // 判断是否退出
        if (strcmp(msg.text, "#") == 0)
        {
            cout << "***********";
            break;
        }

        // 将信息转化成网络字节序
        msg.networkByteOrder();

        if (send(sockfd_, &msg, sizeof(msg), 0) < 0)
        {
            perror("发送信息失败");
            return;
        }

        if(recv(sockfd_,&msg,sizeof(msg),0)<=0)
        {
            perror("接受查询失败");
            return ;
        }

        msg.hostByteOrder();        //转化为本机字节序
        cout<<"释义："<<msg.text<<endl;
    }
}


void DictClient::doHistory()
{
    Msg msg;
    msg.type=H;
    strcpy(msg.name,username_.c_str());

    // 转化为网络字节序，发送给服务器，等待服务器响应
    msg.networkByteOrder();
    if(send(sockfd_,&msg,sizeof(msg),0)<0)
    {
        perror("发送失败");
        return;
    }

    cout<<"历史记录："<<endl;
    // 循环读取信息，防止漏读
    while(true)
    {
        if(recv(sockfd_,&msg,sizeof(msg),0)<=0)
        {
             perror("接收历史数据失败");
            return;
        }
        // 成功接受到信息之后，转化为本机字节序
        msg.hostByteOrder();
        if(strcmp(msg.text, "**OVER**")==0)
        {
            break;
        }
        cout<<msg.text<<endl;

    }
}


// 退出登录功能
void DictClient::doQuit()
{
    if(is_logged_in_)       //判断是否在线
    {
        Msg msg;
        msg.type=Q;

        strcpy(msg.name,username_.c_str());
        msg.networkByteOrder();
        if(send(sockfd_,&msg,sizeof(msg),0)<0)
        {
            perror("发送失败");
            return;
        }else{
            is_logged_in_=0;
            // 清除上机名称
            username_.clear();
            cout<<"用户已经退出"<<endl;
        }
    }
}

