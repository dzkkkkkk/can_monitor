#include "ProcessingPipeline.hpp"
#include <iomanip>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"  // 添加这个头文件

ProcessingPipeline::ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                                       std::unique_ptr<SignalDecoder> decoder)
    : dataSource_(std::move(dataSource)), decoder_(std::move(decoder)) {
    // 初始化日志
    logger_ = spdlog::create<spdlog::sinks::stdout_color_sink_mt>("pipeline");
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
}

ProcessingPipeline::~ProcessingPipeline() {
    stop();
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
    
    for (int i = 0; i < numFrames && !stopRequested_; ++i) {
        CanFrame frame = dataSource_->getNextFrame();
        
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            frameQueue_.push(frame);
            framesProcessed_++;
        }
        
        dataCondition_.notify_one();
        
        logger_->debug("生产帧: ID={:03X} ({}/{})", 
                      frame.id, i+1, numFrames);
    }
    
    logger_->info("生产者线程完成");
    stopRequested_ = true;
    dataCondition_.notify_all();
}

void ProcessingPipeline::consumerThreadFunc() {
    logger_->info("消费者线程启动");
    
    while (!stopRequested_) {
        CanFrame frame;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            dataCondition_.wait(lock, [this] {
                return !frameQueue_.empty() || stopRequested_;
            });
            
            if (stopRequested_ && frameQueue_.empty()) break;
            
            if (!frameQueue_.empty()) {
                frame = frameQueue_.front();
                frameQueue_.pop();
            } else {
                continue;
            }
        }
        
        processFrame(frame);
        framesConsumed_++;
        logger_->debug("消费帧: ID={:03X} (队列大小: {})", 
                      frame.id, frameQueue_.size());
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