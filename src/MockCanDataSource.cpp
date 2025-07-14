#include "MockCanDataSource.hpp"
#include <chrono>
#include <thread>

// 构造函数初始化随机数生成器
MockCanDataSource::MockCanDataSource() 
    : rng(std::random_device{}()),
      idDist(minCanId, maxCanId),
      dataDist(0, 0xFF) {
    setFrameRate(10); // 默认10帧/秒
}

void MockCanDataSource::setFrameRate(unsigned framesPerSec) {
    frameIntervalUs = (framesPerSec > 0) ? (1000000 / framesPerSec) : 100000;
}

void MockCanDataSource::setIdRange(uint32_t minId, uint32_t maxId) {
    minCanId = minId;
    maxCanId = maxId;
    idDist = std::uniform_int_distribution<uint32_t>(minId, maxId);
}

void MockCanDataSource::generateRandomFrame() {
    // 生成随机的CAN ID
    currentFrame.id = idDist(rng);
    
    // 随机DLC(1-8字节)
    std::uniform_int_distribution<uint8_t> dlcDist(1, 8);
    currentFrame.dlc = dlcDist(rng);
    
    // 随机生成数据
    for(uint8_t i = 0; i < currentFrame.dlc; i++) {
        currentFrame.data[i] = dataDist(rng);
    }
    
    // 用递增计数器代替真实时间戳
    currentFrame.timestamp = ++frameCounter * frameIntervalUs;
}

CanFrame MockCanDataSource::getNextFrame() {
    // 模拟帧间隔时间
    if(lastFrameTime > 0) {
        auto sleepTime = frameIntervalUs - (std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() - lastFrameTime);
        
        if(sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
        }
    }
    
    // 生成新的随机帧
    generateRandomFrame();
    
    // 更新时间戳
    lastFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    return currentFrame;
}