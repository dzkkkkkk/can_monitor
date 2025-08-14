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
    src/main.cpp
    src/MockCanDataSource.cpp
    src/SignalDecoder.cpp
    src/ProcessingPipeline.cpp
)

# 链接线程库
target_link_libraries(can_monitor PRIVATE Threads::Threads)

# 添加spdlog日志库 异步日志依赖添加
add_subdirectory(third_party/spdlog)
target_link_libraries(can_monitor PRIVATE
    Threads::Threads
    spdlog::spdlog_header_only
)

# 设置日志宏
target_compile_definitions(can_monitor PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)