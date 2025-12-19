#include "client.hpp"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, const char *argv[])
{
    // 判读传入数据是否足够
    if (argc != 3)
    {
        cerr << "用法：" << argv[0] << " <服务器ip> <端口号>" << endl;
        return -1;
    }

    // 实例化客户端对象
    try
    {
        DictClient client(argv[1], atoi(argv[2]));
        client.run();
    }
    catch (const exception &e)
    {
        cerr << e.what() << "\n";
    }
    std::cout << "Hello, World!" << std::endl;
    return 0;
}