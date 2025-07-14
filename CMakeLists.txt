cmake_minimum_required(VERSION 3.10)
project(CanMonitor LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(include)

# 查找Threads包
find_package(Threads REQUIRED)

# 添加可执行文件
add_executable(can_monitor
    src/ProcessingPipeline.cpp
    src/SignalDecoder.cpp
    src/main.cpp
    src/MockCanDataSource.cpp
)

# 链接线程库
target_link_libraries(can_monitor PRIVATE Threads::Threads)