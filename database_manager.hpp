#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

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

class DatabaseManger
{
    public:
    // 构造函数，需要外部传入用户库的路径和单词库的路径
    DatabaseManger(const string &usr_db_path,const string &dict_db_path);
    ~DatabaseManger();

    // 初始化数据表
    bool initalizeDatabase();

    // 用户相关操作
    // 用户注册操作
    bool registerUser(const string &name,const string &password);

    // 用户登入操作
    bool loginUser(const string &name,const string &password,bool is_online);


    // 用户退出操作
    bool logoutUser(const string &name);


    // 单词表操作
    // 查询单词
    bool querryWord(const string &word,string &meaning);

    // 历史记录
    bool recordHistory(const string &name,const string &word,const string &meaning,const string &time);
    bool getHistory(const string &name ,string &History);

    private:
    sqlite3* usr_db_;       //用户数据库指针
    sqlite3* dict_db_;      //单词数据库指针
    mutex usr_mutex_;       //用户库互斥锁
    mutex dict_mutex_;        // 单词库互斥锁
    
    // 定义私有化函数
    // 初始化用户表
    bool initalizeUserDB();

    // 初始化单词表
    bool initalizeDictDB();

    // 导入单词库
    bool dictToDatabase();

    // 执行sql语句函数
    bool executeSQL(sqlite3 *ppDB,const string &sql,char **errmsg=NULL);

};



#endif  //DATABASE_MANAGER_H