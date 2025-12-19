#include"client.hpp"
#include<bits/stdc++.h>
#include<vector>
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


/*主程序循环*/
void DictClient::run()
{
    while(true)
    {
        showMainMenu();             //显示主菜单
        int choice;                 //选择功能
        cin>>choice;
        cin.ignore();               //清除缓存区


        switch (choice)
        {
        case R:
            doRegister();           //  调用登入功能
            break;
        case L:
            if(doLogin())           //调用登录功能
            {
                showUserMenu();     //调用用户界面
            }
            break;
        case Q:
            doQuit();               //调用退出界面
            break;
        default:    cout<<"无效的选择"<<endl;
            break;
        }  
        cout<<"按回车继续。。。";
        cin.get();              //吸收回车     
    }

}

   
   
void DictClient::showMainMenu()
{
    system("clear");
     cout<<"***************************"<<endl;
    cout<<"*********1、注册***********"<<endl;
    cout<<"*********2、登录***********"<<endl;
    cout<<"*********3、退出***********"<<endl;
    cout<<"***************************"<<endl;
    cout<<"请选择：";
}

void DictClient::showUserMenu()
{
    while(is_logged_in_)
    {
        system("clear");           //清屏
        cout<<"当前用户："<<username_<<endl;
        cout<<"*********************************"<<endl;
        cout<<"**********1、查单词***************"<<endl;
        cout<<"**********2、历史记录***************"<<endl;
        cout<<"**********3、返回上一级***************"<<endl;
        cout<<"*********************************"<<endl;
        cout<<"请选择：";
        int choice;
        cin>>choice;
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
        
        default:     cout<<"无效的选择"<<endl;
            break;
        }
        cout<<"按回车继续...."<<endl ;
        cin.get();
   }
}


// 用户注册函数
bool DictClient::doRegister()
{
    // 定义相关数据信息
}