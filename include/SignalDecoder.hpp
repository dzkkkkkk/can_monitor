#pragma once
#include "MockCanDataSource.hpp"  // 包含CanFrame定义
#include <string>

// 解码后的信号结构
struct DecodedSignal {
    std::string name;        // 信号名称 (如"VehicleSpeed")
    double physicalValue;     // 物理值 (如62.5 km/h)
    std::string unit;         // 单位 (如"km/h")
    uint64_t timestamp;       // 原始数据时间戳
};

class SignalDecoder {
public:
    SignalDecoder();
    
    // 硬编码解析规则
    DecodedSignal decodeSpeed(const CanFrame& frame) const;
    DecodedSignal decodeRPM(const CanFrame& frame) const;
    
    // 通用解析接口（为后续DBC解析预留）
    // std::vector<DecodedSignal> decodeFrame(const CanFrame& frame);
    

    // 信号ID常量（公开访问）
    static constexpr uint32_t SPEED_ID = 0x100;
    static constexpr uint32_t RPM_ID = 0x101;
};