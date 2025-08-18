# can_server.py
import socket
import time
import struct
import random

HOST = '0.0.0.0'
PORT = 8888

def create_can_frame(id, dlc, data, timestamp):
    # 确保数据长度与DLC一致
    data = data[:dlc] + bytes(8 - dlc)
    return struct.pack('I B 8B Q', id, dlc, *data, timestamp)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"CAN 模拟服务器监听在 {HOST}:{PORT}")
    
    conn, addr = s.accept()
    print(f"连接来自 {addr}")
    
    try:
        frame_count = 0
        while True:
            # 生成随机CAN帧
            can_id = 0x100 if random.random() > 0.5 else 0x101
            dlc = random.randint(1, 8)
            data = bytes([random.randint(0, 255) for _ in range(dlc)])
            timestamp = int(time.time() * 1000000)  # 微秒
            
            frame = create_can_frame(can_id, dlc, data, timestamp)
            conn.sendall(frame)
            frame_count += 1
            
            if frame_count % 10 == 0:
                print(f"已发送 {frame_count} 帧")
            
            time.sleep(0.05)  # 约20帧/秒
    except KeyboardInterrupt:
        print("\n服务器关闭")
    except BrokenPipeError:
        print("客户端断开连接")
    finally:
        conn.close()