import serial
import cv2
import numpy as np
import serial.tools.list_ports
import time


SerialPort = "COM4"
 
# 列出可用的串口
ports = list(serial.tools.list_ports.comports())
for port in ports:
    print(port)

# 设置串口参数
ser = serial.Serial(SerialPort, 115200, timeout=10)

# 图像尺寸
width, height = 160, 120

# 接收图像数据
rgb565_data = bytearray()

# 总接收字节计数器
total_bytes_received = 0

# 图像编号，保存的文件名
image_counter = 0



# 图像处理：调整亮度与对比度
def adjust_brightness_contrast(image, alpha=1.1, beta=10):
    # 调整亮度和对比度（适度调整）
    return cv2.convertScaleAbs(image, alpha=alpha, beta=beta)

# 图像处理：去噪（适度去噪）
def denoise_image(image):
    # 使用较轻的中值滤波去噪，保留更多细节
    image = cv2.medianBlur(image, 3)
    return image

# 图像处理：锐化（减少锐化强度）
def sharpen_image(image):
    kernel = np.array([[0, -0.2, 0], [-0.2, 1.8, -0.2], [0, -0.2, 0]])  # 锐化卷积核，减少锐化强度
    return cv2.filter2D(image, -1, kernel)

while True:
    byte = ser.read()  # 读取一个字节
    if not byte:
        continue
    
    rgb565_data += byte
    
    # 更新接收到的字节数
    total_bytes_received += 1

    # 每接收100字节打印一次
    if total_bytes_received % 100 == 0:
        print(f"Total bytes received: {total_bytes_received}")
    
    # 检查是否接收到完整的图像数据（160 * 120 * 2字节，RGB565格式）
    if len(rgb565_data) == width * height * 2:
        print("Complete RGB565 data received")
        
        # 将接收到的 RGB565 数据转换为图像
        img_array = np.frombuffer(rgb565_data, dtype=np.uint16).reshape((height, width))

        # RGB565 to BGR转换
        bgr_img = np.zeros((height, width, 3), dtype=np.uint8)
        bgr_img[..., 0] = np.round((img_array & 0x1F) * (255.0 / 31)).astype(np.uint8)  # Blue channel
        bgr_img[..., 1] = np.round(((img_array >> 5) & 0x3F) * (255.0 / 63)).astype(np.uint8)  # Green channel
        bgr_img[..., 2] = np.round(((img_array >> 11) & 0x1F) * (255.0 / 31)).astype(np.uint8)  # Red channel

        # 处理图像：调整亮度与对比度
        bgr_img = adjust_brightness_contrast(bgr_img)

        # 处理图像：去噪（使用中值滤波，减少噪声）
        bgr_img = denoise_image(bgr_img)

        # 处理图像：锐化（适度锐化）
        bgr_img = sharpen_image(bgr_img)

        # 保存图像为 .png 文件
        image_counter += 1
        image_filename = f"D:/python code/image_rgb565_{image_counter}.png"
        cv2.imwrite(image_filename, bgr_img)
        print(f"Image saved as {image_filename}")

        # 显示图像
        cv2.imshow("OV2640 Image", bgr_img)

        # 等待用户按下 'q' 键退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        # 清空接收到的数据缓存，准备接收下一张图像
        rgb565_data = bytearray()

# 关闭串口
ser.close()
# 销毁所有 OpenCV 窗口
cv2.destroyAllWindows()

