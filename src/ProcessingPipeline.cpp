#include "ProcessingPipeline.hpp"
#include <iomanip>
#include <chrono>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

ProcessingPipeline::ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                                       std::unique_ptr<SignalDecoder> decoder)
    : dataSource_(std::move(dataSource)), decoder_(std::move(decoder)) {
    // 获取全局异步日志器（关键修改）
    logger_ = spdlog::get("async_logger");
    if (!logger_) {
        logger_ = spdlog::default_logger(); // 后备方案
    }
    // 移除格式设置（已在main中统一设置）
}

ProcessingPipeline::~ProcessingPipeline() {
    stop();
    if(logger_) logger_->flush();  // 确保所有日志写入
}

void ProcessingPipeline::stop() {
    stopRequested_ = true;
    dataCondition_.notify_all();
    
    if (producerThread_.joinable()) producerThread_.join();
    if (consumerThread_.joinable()) consumerThread_.join();
}

void ProcessingPipeline::run(int numFrames) {
    logger_->info("启动多线程处理管道");
    logger_->info("目标帧数: {}", numFrames);
    
    stopRequested_ = false;
    framesProcessed_ = 0;
    framesConsumed_ = 0;
    
    auto start_time = std::chrono::steady_clock::now();
    
    // 启动生产者和消费者线程
    producerThread_ = std::thread(&ProcessingPipeline::producerThreadFunc, this, numFrames);
    consumerThread_ = std::thread(&ProcessingPipeline::consumerThreadFunc, this);
    
    // 等待线程结束
    producerThread_.join();
    consumerThread_.join();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    logger_->info("处理完成! 耗时: {}ms", duration.count());
    logger_->info("性能统计 - 生产帧: {}, 消费帧: {}", 
                 framesProcessed_.load(), framesConsumed_.load());
}

void ProcessingPipeline::producerThreadFunc(int numFrames) {
    logger_->info("生产者线程启动");
    constexpr int BUFFER_SIZE = 32; // 缓冲区大小（可根据需求调整）
    
    int currentBuffer = 0; // 生产者使用的当前缓冲区索引

    for (int i = 0; i < numFrames && !stopRequested_; ++i) {
        CanFrame frame = dataSource_->getNextFrame();
        
        // 获取当前缓冲区的独占访问
        {
            std::lock_guard<std::mutex> lock(bufferMutex_[currentBuffer]);
            frameBuffers_[currentBuffer].push_back(frame);
        }
        
        framesProcessed_++;
        logger_->debug("生产帧: ID={:03X} ({}/{})", 
                      frame.id, i+1, numFrames);
        
        // 检查是否达到缓冲区大小或最后一帧
        if (frameBuffers_[currentBuffer].size() >= BUFFER_SIZE || i == numFrames - 1) {
            // 锁定交换操作
            std::unique_lock<std::mutex> swapLock(swapMutex_);
            
            // 交换缓冲区：将填充好的缓冲区移交给消费者
            readyBufferIndex_.store(currentBuffer);
            logger_->info("缓冲区 {} 已准备就绪 ({}帧)", 
                         currentBuffer, frameBuffers_[currentBuffer].size());
            
            // 通知消费者有新数据
            dataCondition_.notify_one();
            
            // 等待消费者处理完成（可选，根据需求决定）
            // consumeCondition_.wait(swapLock, [&]{
            //     return frameBuffers_[currentBuffer].empty() || stopRequested_;
            // });
            
            // 切换到另一个缓冲区继续填充
            currentBuffer = 1 - currentBuffer;
        }
    }
    
    // 最终通知
    stopRequested_ = true;
    dataCondition_.notify_all();
    logger_->info("生产者线程完成");
}

void ProcessingPipeline::consumerThreadFunc() {
    logger_->info("消费者线程启动");
    
    while (!(stopRequested_==true&&readyBufferIndex_==-1)) { //只有当停止符为真而且当前缓冲区已经处理完
        int readyIndex = -1;
        std::vector<CanFrame> framesToProcess;
        
        {
            std::unique_lock<std::mutex> swapLock(swapMutex_);
            
            // 等待就绪缓冲区或停止信号
            dataCondition_.wait(swapLock, [this] {
                return readyBufferIndex_ != -1 || stopRequested_;
            });
            
            if (stopRequested_&&readyBufferIndex_==-1) break;//添加了readyBufferIndex_防止最后一个缓冲区没来得及消费就结束了
            
            // 获取就绪缓冲区索引
            readyIndex = readyBufferIndex_.load();
            readyBufferIndex_.store(-1);  // 重置就绪状态
            
            // 转移缓冲区内容
            {
                std::lock_guard<std::mutex> bufLock(bufferMutex_[readyIndex]);
                framesToProcess = std::move(frameBuffers_[readyIndex]);
                frameBuffers_[readyIndex].clear();
            }
            
            logger_->info("开始处理缓冲区 {} ({}帧)", 
                         readyIndex, framesToProcess.size());
        }
        
        // 处理所有帧
        for (auto& frame : framesToProcess) {
            processFrame(frame);
            framesConsumed_++;
        }
        
        logger_->info("完成处理缓冲区 {} (已处理{}帧)", 
                     readyIndex, framesToProcess.size());
        
        
        // 通知生产者缓冲区已清空（如果启用了等待）
        // consumeCondition_.notify_one();
    }
    
    logger_->info("消费者线程完成");
}

void ProcessingPipeline::processFrame(const CanFrame& frame) {
    // 使用日志替代原始输出
    if (frame.id == SignalDecoder::SPEED_ID) {
        auto speed = decoder_->decodeSpeed(frame);
        if (!std::isnan(speed.physicalValue)) {
            logger_->info("车速: {:.1f} km/h", speed.physicalValue);
        } else {
            logger_->warn("无效车速信号! ID: {:03X}, DLC: {}", frame.id, frame.dlc);
        }
    } else if (frame.id == SignalDecoder::RPM_ID) {
        auto rpm = decoder_->decodeRPM(frame);
        if (!std::isnan(rpm.physicalValue)) {
            logger_->info("转速: {:.0f} RPM", rpm.physicalValue);
        } else {
            logger_->warn("无效转速信号! ID: {:03X}, DLC: {}", frame.id, frame.dlc);
        }
    } else {
        logger_->debug("未识别帧: ID={:03X}", frame.id);
    }
}