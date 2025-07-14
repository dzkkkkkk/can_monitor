#include <iostream>
#include <iomanip>
#include "MockCanDataSource.hpp"

// 打印CAN帧的辅助函数
void printCanFrame(const CanFrame& frame) {
    std::cout << std::dec << frame.timestamp << " μs | "
              << std::hex << std::setw(3) << std::setfill('0') 
              << frame.id << " | ["
              << static_cast<int>(frame.dlc) << "] ";
              
    for(int i = 0; i < frame.dlc; i++) {
        std::cout << std::hex << std::setw(2) 
                  << static_cast<int>(frame.data[i]) << " ";
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "CAN Monitor启动成功!" << std::endl;
    std::cout << "系统版本: " << __cplusplus << std::endl;
    
    // 创建并配置数据源
    MockCanDataSource dataSource;
    dataSource.setFrameRate(5); // 5帧/秒
    dataSource.setIdRange(0x100, 0x500); // 限制ID范围
    
    std::cout << "模拟数据源配置: 5帧/秒, ID范围: 0x100-0x500" << std::endl;
    std::cout << "时间戳(μs) | ID | [DLC] 数据" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    // 获取并打印10帧数据
    for(int i = 0; i < 10; i++) {
        CanFrame frame = dataSource.getNextFrame();
        printCanFrame(frame);
    }
    
    return 0;
}