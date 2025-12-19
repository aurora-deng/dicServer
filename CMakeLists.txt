cmake_minimum_required(VERSION 3.15)
project(dict_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找sqlite3（客户端可能不需要，为未来扩展做准备）

find_package(SQLite3)

# 找到线程库
find_package(Threads REQUIRED)

# 添加可执行文件
add_executable(${project_name}
    client.cpp
    main.cpp
    network.cpp
)


# 包含当前目录
target_include_directories(${project_name} private ${cmake_current_source_dir})


# 链接库
target_link_libraries(${project_name} private threads::threads)

# 设置调试编译选项
if(camke_build_type strequal "Debug")
                target_compile_options(${project_name} private -Wall -wextra -g)
endif()