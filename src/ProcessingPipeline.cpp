#include "ProcessingPipeline.hpp"
#include <iomanip>
#include <iostream>

ProcessingPipeline::ProcessingPipeline(std::unique_ptr<MockCanDataSource> dataSource,
                                       std::unique_ptr<SignalDecoder> decoder)
    : dataSource_(std::move(dataSource)), decoder_(std::move(decoder)) {}

void ProcessingPipeline::run(int numFrames) {
    for (int i = 0; i < numFrames; ++i) {
        // 1. 获取数据帧
        CanFrame frame = dataSource_->getNextFrame();
        
        // 2. 处理帧
        processFrame(frame);
    }
}

void ProcessingPipeline::processFrame(const CanFrame& frame) {
    // 打印原始帧
    std::cout << "原始帧: " 
              << std::hex << std::setw(3) << std::setfill('0') << frame.id
              << " | [" << static_cast<int>(frame.dlc) << "] ";
    for (int i = 0; i < frame.dlc; i++) {
        std::cout << std::hex << std::setw(2) 
                  << static_cast<int>(frame.data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    // 尝试解析信号
    if (frame.id == SignalDecoder::SPEED_ID) {
        auto speed = decoder_->decodeSpeed(frame);
        if (!std::isnan(speed.physicalValue)) {
            std::cout << "  车速: " << std::fixed << std::setprecision(1)
                      << speed.physicalValue << " " << speed.unit << std::endl;
        }
    } else if (frame.id == SignalDecoder::RPM_ID) {
        auto rpm = decoder_->decodeRPM(frame);
        if (!std::isnan(rpm.physicalValue)) {
            std::cout << "  转速: " << std::fixed << std::setprecision(0)
                      << rpm.physicalValue << " " << rpm.unit << std::endl;
        }
    }
}