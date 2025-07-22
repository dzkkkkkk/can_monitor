#pragma once
#include "MockCanDataSource.hpp"
#include "SignalDecoder.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <array>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "spdlog/logger.h"

class ProcessingPipeline {
public:
    ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                       std::unique_ptr<SignalDecoder> decoder);
    ~ProcessingPipeline();
    
    void stop();
    void run(int numFrames);
    
private:
    // 多线程组件
    std::thread producerThread_;
    std::thread consumerThread_;
    std::mutex swapMutex_;                // 保护缓冲区交换操作
    std::array<std::mutex, 2> bufferMutex_; // 保护各个缓冲区
    std::condition_variable dataCondition_;
    std::condition_variable consumeCondition_; // 可选的生产者等待
    
    // 双缓冲队列
    std::array<std::vector<CanFrame>, 2> frameBuffers_;
    std::atomic<int> readyBufferIndex_{-1}; // 就绪缓冲区索引（-1表示无就绪缓冲区）

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