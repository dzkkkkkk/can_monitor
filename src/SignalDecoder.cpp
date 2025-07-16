#include "SignalDecoder.hpp"
#include <cmath>  // 用于isnan检查

SignalDecoder::SignalDecoder() {
    // 初始化代码（后续添加）
}

// 解析车速信号
// 假设：ID=0x100，数据前2字节（大端序）表示车速，单位0.1 km/h
DecodedSignal SignalDecoder::decodeSpeed(const CanFrame& frame) const {
    DecodedSignal signal;
    signal.name = "VehicleSpeed";
    signal.unit = "km/h";
    signal.timestamp = frame.timestamp;
    
    if (frame.id == SPEED_ID && frame.dlc >= 2) {
        // 大端序解析：第一个字节是高位，第二个字节是低位
        uint16_t raw = (static_cast<uint16_t>(frame.data[0]) << 8) | frame.data[1];
        signal.physicalValue = raw * 0.001;  // 转换为km/h
    } else {
        signal.physicalValue = NAN;  // 使用NaN表示无效值
    }
    return signal;
}

// 解析发动机转速
// 假设：ID=0x101，数据第3、4字节（小端序）表示转速，单位0.25 RPM
DecodedSignal SignalDecoder::decodeRPM(const CanFrame& frame) const {
    DecodedSignal signal;
    signal.name = "EngineRPM";
    signal.unit = "RPM";
    signal.timestamp = frame.timestamp;
    
    if (frame.id == RPM_ID && frame.dlc >= 4) {
        // 小端序解析：第三个字节是低位，第四个字节是高位
        uint16_t raw = (static_cast<uint16_t>(frame.data[3]) << 8) | frame.data[2];
        signal.physicalValue = raw * 0.25;
    } else {
        signal.physicalValue = NAN;
    }
    return signal;
}