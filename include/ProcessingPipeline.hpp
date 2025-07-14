#pragma once
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include <vector>
#include <memory>

class ProcessingPipeline {
public:
    ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                       std::unique_ptr<SignalDecoder> decoder);
    
    void run(int numFrames);
    
private:
    // 数据源组件
    std::unique_ptr<MockCanDataSource> dataSource_;
    
    // 信号解析器
    std::unique_ptr<SignalDecoder> decoder_;
    
    // 处理CAN帧
    void processFrame(const CanFrame& frame);
};