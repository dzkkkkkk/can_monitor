#pragma once
#include <cstdint>
#include <random>  // 添加随机数支持

struct CanFrame {
    uint32_t id;        // CAN ID (11位或29位)
    uint8_t dlc;        // 数据长度码 (0-8)
    uint8_t data[8];    // 数据负载
    uint64_t timestamp; // 时间戳(微秒)
};

class MockCanDataSource {
public:
    MockCanDataSource();
    
    // 配置模拟参数
    void setFrameRate(unsigned framesPerSec);    // 帧生成速率
    void setIdRange(uint32_t minId, uint32_t maxId); // ID范围
    
    // 获取下一帧数据
    CanFrame getNextFrame();
    
private:
    void generateRandomFrame();
    
    // 模拟状态
    uint32_t frameCounter = 0;
    uint64_t lastFrameTime = 0;
    
    // 配置参数
    unsigned frameIntervalUs; // 帧间隔(微秒)
    uint32_t minCanId = 0x100;
    uint32_t maxCanId = 0x7FF;
    
    // 随机数生成器
    std::mt19937 rng;
    std::uniform_int_distribution<uint32_t> idDist;
    std::uniform_int_distribution<uint8_t> dataDist;
    
    // 当前帧数据
    CanFrame currentFrame;
};