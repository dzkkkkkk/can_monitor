#include <iostream>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"  // 添加异步日志头文件
#include "spdlog/sinks/stdout_color_sinks.h"
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include "ProcessingPipeline.hpp"

int main() {
    // 初始化异步日志系统 - 关键修改
    const size_t queue_size = 8192;  // 日志队列大小
    spdlog::init_thread_pool(queue_size, 1);  // 1个后台线程
    
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger", 
        console_sink, 
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );
    
    spdlog::set_default_logger(async_logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);
    
    spdlog::info("CAN Monitor启动成功! (异步日志模式)");
    spdlog::info("系统版本: {}", __cplusplus);
    
    // 创建并配置数据源
    auto dataSource = std::make_unique<MockCanDataSource>();
    dataSource->setFrameRate(20); // 20帧/秒
    dataSource->setIdRange(0x100, 0x101); //范围
    
    // 创建信号解析器
    auto decoder = std::make_unique<SignalDecoder>();
    
    // 创建处理管道
    auto pipeline = std::make_unique<ProcessingPipeline>(
        std::move(dataSource),
        std::move(decoder)
    );
    
    // 运行管道处理100帧
    pipeline->run(100);
    
    spdlog::info("双缓冲队列修复");
    spdlog::info("程序退出");
    return 0;
}