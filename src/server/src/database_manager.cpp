#include "database_manager.hpp"
#include <fstream>   //IO操作的头文件
#include <ctime>     //时间函数
#include <stdexcept> //异常处理头文件
#define DICT_PATH "./dict.txt"

using namespace std;

/*
构造函数：打开数据库连接
参数1：用户数据库的文件路径
参数2：词典数据库的文件路径
返回值：如果数据库打开失败，则返回异常处理：runtime_error
*/

DatabaseManger::DatabaseManger(const string &user_db_path, const string &dic_db_path)
{
    // 打开或创建用户数据库
    if (sqlite3_open(user_db_path.c_str(), &usr_db_) != SQLITE_OK)
    {
        throw runtime_error("用户数据打开失败：" + string(sqlite3_errmsg(usr_db_)));
    }
    // 打开或者创建数据库
    if (sqlite3_open(dic_db_path.c_str(), &dict_db_) != SQLITE_OK)
    {
        throw runtime_error("单词库数据打开失败：" + string(sqlite3_errmsg(dict_db_)));
    }
}

DatabaseManger::~DatabaseManger()
{
    sqlite3_close(usr_db_);  // 关闭用户数据库
    sqlite3_close(dict_db_); // 关闭单词数据库
}

/*
初始化数据库，创建相关数据表，并导入单词库
*/

bool DatabaseManger::initalizeDatabase()
{
    // 用户表和单词表都成功后，表示数据库初始化成功
    return initalizeUserDB() && initalizeDictDB();
}

/*
初始化用户表：创建一个用户表
*/

bool DatabaseManger::initalizeUserDB()
{
    // 使用互斥锁锁住数据库的操作，防止出现多线程竞态
    lock_guard<mutex> lock(usr_mutex_);

    // 准备sql语句：创建用户表和历史记录sql语句
    const char *sql = "create table if not exists usr("     // 创建用户表
                      "name text primary key,"              // 用户名（主键）
                      "password int,"                       // 密码
                      "stage int);"                         // 用户状态：0表示离线，1表示在线
                      "create table if not exists history(" // 创建历史记录表
                      "name text,"                          // 用户名
                      "word text,"                          // 查询单词
                      "mean text,"                          // 单词意思
                      "time text);";                        // 查询时间

    // 执行sql语句
    if (!executeSQL(usr_db_, sql))
    {
        cout << "数据表初始化失败" << endl;
        return false;
    }

    // 重置所有的客户都为离线状态
    sql = "update usr set stage=0;";
    if (!executeSQL(usr_db_, sql))
    {
        cout << "用户状态重置失败" << endl;
        return false;
    }

    return true; // 表示两个数据表创建成功，并且将所有客户端设置成离线状态
}

bool DatabaseManger::initalizeDictDB()
{
    // 创建互斥锁，保护单词表
    lock_guard<mutex> lock(dict_mutex_);

    // 准备创建单词数据库的sql语句
    const char *sql = "create table if not exists dict(word text,mean text);";
    if (!executeSQL(dict_db_, sql))
    {
        cerr << "单词表创建失败" << endl;
        return false;
    }

    // 检查词典数据库是否为空
    sql = "select * from dict;"; // 查询数据库中的记录个数
    sqlite3_stmt *stmt;          // 接受预处理sql的结果

    /*
    将上面的sql预编译一下
    函数介绍：将sql语句进行预编译一遍，用于后面的执行
    参数1：数据库句柄
    参数2：要编译的sql语句
    参数3：-1表示sql是一个字符串
    参数4：编译后的结果存放的位置
    参数5：接收没有处理的剩余的sql语句
    */
    if (sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
        return false;
    }

    bool need_import = false; // 标识是否需要导入数据

    // 执行sql语句
    if (sqlite3_step(stmt) == SQLITE_OK) // 表示执行成功
    {
        if (sqlite3_column_int(stmt, 0) == 0) // 表示数据为空
        {
            // 表示表为空，需要导入数据
            need_import = true;
        }
    }

    // 释放stmt空间
    // 功能：是否预编译sql语句的空间
    sqlite3_finalize(stmt);

    // 如果需要导入，返回导入函数结果，如果已有数据表结果，则直接返回
    return need_import ? dictToDatabase() : true;
}

/*
将单词文本的信息导入到数据库中
*/

bool DatabaseManger::dictToDatabase()
{
    // 使用文件操作，打开文件dict.txt
    ifstream file(DICT_PATH);
    if (!file)
    {
        cerr << "无法打开词典文件:" << DICT_PATH << endl;
        return false;
    }

    string line;                // 用于读取文件中的一行数据
    while (getline(file, line)) // 循环读取file中的一行信息放入到line中
    {
        // 跳过空行
        if (line.empty())
            continue;

        // 分割单词和意义 格式（单词 意义）
        size_t pos = line.find(' '); // 定位到空格的位置
        if (pos == string::npos)
        {
            cerr << "无效词典条目（确实空格分隔符）" << line << endl;
            continue;
        }

        // 将单词和意义分开储存
        string word = line.substr(0, pos);
        string mean = line.substr(pos + 1);

        // 准备sql语句可以使用通配符表示
        const char *sql = "insert into dict values(?,?)";

        // 预编译sql语句，
        sqlite3_stmt *stmt = NULL;

        if (sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
            file.close();
            return false;
        }

        // 预编译成功，可以为已经编译成功的内容绑定参数
        sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, mean.c_str(), -1, SQLITE_STATIC);

        // 执行插入语句
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "数据插入失败：" << sqlite3_errmsg(dict_db_) << endl;
            sqlite3_finalize(stmt);
            file.close();
            return false;
        }

        // 程序执行到此，表示该单词已经导入到数据库中
        sqlite3_finalize(stmt);
    }

    // 程序执行至此，表示，单词表导入成功
    file.close();
    cout << "单词库导入成功" << endl;
    return true;
}

// 执行sql语句函数
/*
参数：数据库语柄，要执行的sql语句，错误信息
*/

bool DatabaseManger::executeSQL(sqlite3 *ppdb, const string &sql, char **errmsg)
{
    if (sqlite3_exec(ppdb, sql.c_str(), NULL, NULL, errmsg) != SQLITE_OK)
    {
        cerr << "sql执行失败：" << *errmsg << endl;
        // 释放错误信息空间
        sqlite3_free(errmsg);
        return false;
    }

    return true; // 表示正常执行了sql语句
}

/*
注册功能；就是将用户名和密码放入到usr表中，如果有重复就报错
参数：用户名，密码
*/

bool DatabaseManger::registerUser(const string &name, const string &password)
{
    // 使用互斥锁防止多线程出现竞态
    lock_guard<mutex> lock(usr_mutex_);

    // 准备sql语句，使用通配符来描述，后面直接绑定即可，全部都初始化为0的离线状态
    const char *sql = "insert into usr values(?,?,0);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql语句执行失败" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    // 预编译成功之后就可以给通配符进行bind
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    // 执行插入操作
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // 对于执行结果进行判断
    if (result == SQLITE_CONSTRAINT)
    {
        // 违反主键唯一性
        cerr << "该用户名已存在；" << name << endl;
        return false;
    }

    // 表示注册成功
    return result = SQLITE_DONE;
}

/*
用户登入功能：
参数：用户名，密码，判断是否在线，防止冲突
*/

bool DatabaseManger::loginUser(const string &name, const string &password, bool is_online)
{
    // 同样加锁
    lock_guard<mutex> lock(usr_mutex_);

    // 准备sql语句
    const char *sql = "select stage from usr where name=? and passeord=?;";

    // 预编译测试
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql准备失败：" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    // 预编译成功，可以为通配符绑定上要真正输入的
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    // 执行编译操作
    int result = sqlite3_step(stmt);

    // 对于执行结果进行判断
    if (result == SQLITE_ROW)
    {
        // 获取用户的状态
        if (!is_online)
        {
            sqlite3_finalize(stmt); // 释放空间

            // 准备sql语句，进行预编译,更新该用户的状态
            const char *update_sql = "update usr set stage=1 where name=?;";

            if (sqlite3_prepare_v2(usr_db_, update_sql, -1, &stmt, NULL) != SQLITE_OK)
            {
                cerr << "sql准备失败" << sqlite3_errmsg(usr_db_) << endl;
                return false;
            }

            // 如果预编译成功，就执行绑定操作

            sqlite3_bind_text(stmt, 1, name.c_str(), 1, SQLITE_STATIC);
            // 更新运行结果
            result = sqlite3_step(stmt);
        }
    }

    // 释放空间
    sqlite3_finalize(stmt);
    return result = SQLITE_DONE;
}

/*
    用户退出功能：将该用户对对应的状态调整成0即可
    参数：用户名
*/

bool DatabaseManger::logoutUser(const string &name)
{
    // 依旧上锁
    lock_guard<mutex> lock(usr_mutex_);

    // 准备sql语句
    const char *sql = "update usr set stage=0 where name=?;";

    // 预编译sql语句
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cout << "sql语句准备失败" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    // 绑定信息
    int result = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    // 执行更新操作
    sqlite3_step(stmt);
    return result == SQLITE_DONE;
}

/*

查单词功能：
    参数：单词，返回意义
*/
bool DatabaseManger::querryWord(const string &word, string &meaning)
{
    // 老样子，多线程上锁
    lock_guard<mutex> lock(dict_mutex_);

    // 准备sql语句
    const char *sql = "select mean from dict where word=?;";
    // 预编译sql语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
        return false;
    }

    // 绑定信息
    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);

    // 执行查询结果
    bool found = false; // 标识是否查询到
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 获取查询结果
        const unsigned char *result = sqlite3_column_text(stmt, 0);
        if (result)
        {
            meaning = reinterpret_cast<const char *>(result); // 转换数据类型
            found = true;
        }
    }

    // 释放空间
    sqlite3_finalize(stmt);
    return found;
}

/*
    历史记录函数：将给定的信息插入到历史数据库中
    参数1：用户名
    参数2：单词
    参数3：含义
    参数4：查询时间
*/

bool DatabaseManger::recordHistory(const string &name, const string &word, const string &meaning, const string &time)
{
    // 依旧使用互斥锁防止竞态
    lock_guard<mutex> lock(usr_mutex_);

    // 准备预编译sql语句
    const char *sql = "insert into history values(?,?,?,?);";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql预编译失败" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    // 绑定信息
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 1, meaning.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 1, time.c_str(), -1, SQLITE_STATIC);

    // 执行插入操作
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result = SQLITE_DONE;
}

/*
    获取查单词的历史记录
    参数1：用户名
    参数2：查询的结果字符串
*/
bool DatabaseManger::getHistory(const string &name, string &History)
{
    lock_guard<mutex> lock(dict_mutex_);

    const char *sql = "select word,meaning,time from history where name=?; order by time DESC";

    // 预编译sql
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cerr << "sql预编译错误" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    // 执行查询语句
    History.clear(); // 防止之前的数据干扰

    while (sqlite3_step(stmt) == SQLITE_OK)
    {
        // 获得该信息的每一列
        const unsigned char *word = sqlite3_column_text(stmt, 0);
        const unsigned char *mean = sqlite3_column_text(stmt, 1);
        const unsigned char *time = sqlite3_column_text(stmt, 2);
        // 将数据变成一条字符串
        if (word && mean && time)
        {
            History+=string(reinterpret_cast<const char *>(word))+"\t";
            History+=string(reinterpret_cast<const char *>(mean))+"\t";
            History+=string(reinterpret_cast<const char *>(time))+"\t";
        }
    }
    sqlite3_finalize(stmt);
    return true;
}