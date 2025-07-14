#include <iostream>
#include <memory>
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include "ProcessingPipeline.hpp"

int main() {
    std::cout << "CAN Monitor启动成功!" << std::endl;
    
    // 创建数据源
    auto dataSource = std::make_unique<MockCanDataSource>();
    dataSource->setFrameRate(5);
    dataSource->setIdRange(0x100, 0x101);
    
    // 创建信号解析器
    auto decoder = std::make_unique<SignalDecoder>();
    
    // 创建处理管道
    auto pipeline = std::make_unique<ProcessingPipeline>(
        std::move(dataSource),
        std::move(decoder)
    );
    
    std::cout << "启动处理管道..." << std::endl;
    pipeline->run(10);
    
    return 0;
}