#include <iostream>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"  // 添加这个头文件
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include "ProcessingPipeline.hpp"

int main() {
    // 初始化日志系统
    auto console = spdlog::create<spdlog::sinks::stdout_color_sink_mt>("console");
    spdlog::set_default_logger(console);
    console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);
    
    spdlog::info("CAN Monitor启动成功!");
    spdlog::info("系统版本: {}", __cplusplus);
    
    // 创建并配置数据源
    auto dataSource = std::make_unique<MockCanDataSource>();
    dataSource->setFrameRate(500); // 20帧/秒
    dataSource->setIdRange(0x100, 0x101); //范围
    
    // 创建信号解析器
    auto decoder = std::make_unique<SignalDecoder>();
    
    // 创建处理管道
    auto pipeline = std::make_unique<ProcessingPipeline>(
        std::move(dataSource),
        std::move(decoder)
    );
    
    // 运行管道处理100帧
    pipeline->run(2000);
    
    spdlog::info("双缓冲队列修复");
    spdlog::info("程序退出");
    return 0;
}