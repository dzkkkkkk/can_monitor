#pragma once
#include <cstdint>  // 添加uint32_t等类型支持

struct CanFrame {
    uint32_t id;        // CAN ID
    uint8_t dlc;        // 数据长度
    uint8_t data[8];    // 数据负载
    uint64_t timestamp; // 时间戳
};

class MockCanDataSource {
public:
    MockCanDataSource();
    CanFrame getNextFrame();
    
private:
    uint32_t frameCounter = 0; // 模拟帧计数器
};

