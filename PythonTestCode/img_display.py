import serial
import numpy as np
import cv2

ser = serial.Serial("COM9", 115200, timeout=1)

while True:
    img_data = ser.read(320 * 240 * 2)  # RGB565 格式
    img = np.frombuffer(img_data, dtype=np.uint8).reshape((240, 320, 2))
    
    # 转换为 BGR
    r = (img[:, :, 0] & 0xF8) << 8
    g = (img[:, :, 0] & 0x07) << 5 | (img[:, :, 1] & 0xE0) >> 3
    b = (img[:, :, 1] & 0x1F) << 3
    img_bgr = np.stack([b, g, r], axis=-1)

    cv2.imshow("Image", img_bgr)
    if cv2.waitKey(1) == 27:  # 按 ESC 退出
        break

ser.close()
cv2.destroyAllWindows()