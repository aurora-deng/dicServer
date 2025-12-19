cmake_minimum_required(VERSION 3.15)
project(dict_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找包
find_package(SQLite3)          # 找不到会继续走，不会 fatal
find_package(Threads REQUIRED)

# 生成可执行文件
add_executable(${PROJECT_NAME}      # ← 全大写
    client.cpp
    main.cpp
    network.cpp
)

# 头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE   # ← 大写 + 关键字
    ${CMAKE_CURRENT_SOURCE_DIR}                      # ← 全大写
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE        # ← 全大写 + 关键字
    Threads::Threads                                 # ← 官方目标名大小写
)

# Debug 附加选项
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if(build_type STREQUAL "debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -g
    )
endif()