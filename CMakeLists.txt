cmake_minimum_required(VERSION 3.12)

project(dict_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 SQLite 和线程库
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# 可执行文件
add_executable(server
    src/main.cpp
    src/server.cpp
    src/database_manager.cpp
)

# 添加头文件目录
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接 SQLite 和线程库
target_link_libraries(server PRIVATE SQLite::SQLite3 Threads::Threads)
