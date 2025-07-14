#pragma once
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "spdlog/logger.h"

class ProcessingPipeline {
public:
    ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                       std::unique_ptr<SignalDecoder> decoder);
    ~ProcessingPipeline();
    
    void stop();  // 添加停止函数声明
    void run(int numFrames);
    
private:
    // 多线程组件
    std::thread producerThread_;
    std::thread consumerThread_;
    std::mutex queueMutex_;
    std::condition_variable dataCondition_;
    std::queue<CanFrame> frameQueue_;
    std::atomic<bool> stopRequested_{false};
    
    // 线程函数
    void producerThreadFunc(int numFrames);
    void consumerThreadFunc();
    
    // 处理单帧
    void processFrame(const CanFrame& frame);
    
    // 日志记录器
    std::shared_ptr<spdlog::logger> logger_;
    
    // 性能统计
    std::atomic<int> framesProcessed_{0};
    std::atomic<int> framesConsumed_{0};
    
    std::unique_ptr<MockCanDataSource> dataSource_;
    std::unique_ptr<SignalDecoder> decoder_;
};