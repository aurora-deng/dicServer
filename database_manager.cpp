#include"database_manager.hpp"
#include<fstream>                           //IO操作的头文件
#include<ctime>                             //时间函数
#include<stdexcept>                         //异常处理头文件
using namespace std;


/*
构造函数：打开数据库连接
参数1：用户数据库的文件路径
参数2：词典数据库的文件路径
返回值：如果数据库打开失败，则返回异常处理：runtime_error
*/


DatabaseManger::DatabaseManger(const string &user_db_path,const string &dic_db_path)
{
    // 打开或创建用户数据库
    if(sqlite3_open(user_db_path.c_str(),&usr_db_)!=SQLITE_OK)
    {
        throw runtime_error("用户数据打开失败："+string(sqlite3_errmsg(dict_db_)));
    }
}

DatabaseManger::~DatabaseManger()
{
    sqlite3_close(usr_db_);          //关闭用户数据库
    sqlite3_close(dict_db_);          //关闭单词数据库
}

/*
初始化数据库，创建相关数据表，并导入单词库
*/

bool DatabaseManger::initalizeDatabase()
{
    //用户表和单词表都成功后，表示数据库初始化成功
    return initalizeUserDB()&&initalizeDictDB();
}

/*
初始化用户表：创建一个用户表
*/

bool DatabaseManger::initalizeUserDB()
{
    // 使用互斥锁锁住数据库的操作，防止出现多线程竞态
    lock_guard<mutex> lock(usr_mutex_);

    // 准备sql语句：创建用户表和历史记录sql语句
    const char *sql="create table if not exists usr("       //创建用户表
                        "name text primary key,"            //用户名（主键）
                        "password int,"                     //密码
                        "stage int);"                       //用户状态：0表示离线，1表示在线    
                        "create table if not exists history("   //创建历史记录表
                        "name text,"                            //用户名
                        "word text,"                            //查询单词
                        "mean text,"                            //单词意思
                        "time text);"  ;                         //查询时间
    
    // 执行sql语句
    if(!executeSQL(usr_db_,sql))
    {
        cout<<"数据表初始化失败"<<endl;
        return false;
    }

    // 重置所有的客户都为离线状态
    sql="update usr set stage=0;";
    if(!executeSQL(usr_db_,sql))
    {
        cout<<"用户状态重置失败"<<endl;
        return false;
    }

    return true;         //表示两个数据表创建成功，并且将所有客户端设置成离线状态
}