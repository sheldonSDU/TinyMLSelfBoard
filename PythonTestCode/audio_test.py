import serial
import wave
import pyaudio
import time
import matplotlib.pyplot as plt
import numpy as np
import struct

# --- 配置参数 ---
SERIAL_PORT = 'COM4'   # 你的串口
BAUD_RATE = 115200     # 还是建议改为 921600，否则太慢了
DURATION_SECONDS = 1
SAMPLE_RATE = 16000
SAMPLE_WIDTH = 2       
CHANNELS_IN = 2        
PCM_SIZE = SAMPLE_RATE * SAMPLE_WIDTH * DURATION_SECONDS * CHANNELS_IN # 64000

OUTPUT_WAV = 'audio_1s_final.wav'

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=None) # 取消硬件超时
    print(f"串口 {SERIAL_PORT} 已打开。")
    print(">>> 请现在按下开发板复位键 (Reset) <<<")
    
    pcm_data_raw = b''
    
    # --- 第一阶段：阻塞等待第一个字节 ---
    # 只要没收到数据，就一直死等，不再会有超时报错
    print("等待数据流开始...")
    while ser.in_waiting == 0:
        time.sleep(0.01)
        
    print("检测到数据流，开始接收...")
    start_time = time.time() # 【关键修改】收到数据才开始计时

    # --- 第二阶段：接收剩余数据 ---
    # 给足 10秒 传输时间（相对于开始接收的时间）
    while len(pcm_data_raw) < PCM_SIZE:
        # 读取缓冲区所有数据
        if ser.in_waiting:
            pcm_data_raw += ser.read(ser.in_waiting)
        
        # 如果超过 10秒 还没传完，说明断了
        if time.time() - start_time > 10:
            print("\n[错误] 传输中途超时！数据流中断。")
            break
        
        time.sleep(0.005) # 极短睡眠避免CPU占用过高

    ser.close()

    print(f"\n接收结束。期望: {PCM_SIZE}, 实际: {len(pcm_data_raw)}")

    if len(pcm_data_raw) < PCM_SIZE:
        print("警告：数据依然不完整，建议检查杜邦线接触或提高波特率。")
        # 即使不完整也继续尝试处理，看看波形
    
    # --- 数据处理：提取有效声道 ---
    # 你的截图中显示右声道能量(1.4亿)远大于左声道(1400万)，所以逻辑是正确的
    total_samples = len(pcm_data_raw) // 2
    shorts_array = struct.unpack(f'<{total_samples}h', pcm_data_raw)
    
    channel_L = shorts_array[0::2]
    channel_R = shorts_array[1::2]

    # 自动判断声道
    if np.sum(np.abs(channel_L)) > np.sum(np.abs(channel_R)):
        print(">>> 锁定左声道")
        mono_data = channel_L
    else:
        print(">>> 锁定右声道")
        mono_data = channel_R

    final_bytes = struct.pack(f'<{len(mono_data)}h', *mono_data)

    # --- 播放 ---
    p = pyaudio.PyAudio()
    stream = p.open(format=p.get_format_from_width(SAMPLE_WIDTH),
                    channels=1, rate=SAMPLE_RATE, output=True)
    stream.write(final_bytes)
    stream.stop_stream()
    stream.close()
    p.terminate()

    # --- 保存 ---
    with wave.open(OUTPUT_WAV, 'wb') as wf:
        wf.setnchannels(1)
        wf.setsampwidth(SAMPLE_WIDTH)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(final_bytes)
    print(f"文件已保存: {OUTPUT_WAV}")

    # --- 绘图 ---
    plt.plot(mono_data)
    plt.title(" Audio Waveform")
    plt.show()

except Exception as e:
    print(f"程序崩溃: {e}")