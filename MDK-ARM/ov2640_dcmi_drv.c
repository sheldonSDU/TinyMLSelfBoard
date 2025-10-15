/**
  ******************************************************************************
  * @file    ov2640_dcmi_drv.c
  * @brief   This file provides code for the utilization
  *          of Camera OV2640
  ******************************************************************************
  * @attention.
  * 	1. The configuration interface is i2c2
	*   2. Refer to :https://blog.csdn.net/qq_40500005/article/details/112055533
  ******************************************************************************
  */

#include "ov2640_dcmi_drv.h"
#include <stdio.h>
#include "gpio.h"

uint8_t frame_buffer[FRAME_SIZE] __attribute__((aligned(32)));

/* I2C 写入函数 */
// 向 OV2640 写入寄存器
HAL_StatusTypeDef OV2640_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;    // 寄存器地址
    data[1] = value;  // 寄存器值

    if (HAL_I2C_Master_Transmit(&hi2c2, OV2640_I2C_ADDRESS, data, 2, HAL_MAX_DELAY) != HAL_OK) {
        // 传输失败处理
        return HAL_ERROR;
    }
    return HAL_OK;
}

/* I2C 读取函数 */
HAL_StatusTypeDef OV2640_ReadReg(uint8_t reg, uint8_t *value) {
    if (HAL_I2C_Master_Transmit(&hi2c2, OV2640_I2C_ADDRESS, &reg, 1, HAL_MAX_DELAY) != HAL_OK) {
        // 传输失败处理
        return HAL_ERROR;
    }
    if (HAL_I2C_Master_Receive(&hi2c2, OV2640_I2C_ADDRESS, value, 1, HAL_MAX_DELAY) != HAL_OK) {
        // 接收失败处理
        return HAL_ERROR;
    }
    return HAL_OK;
}



/* OV2640 摄像头初始化 */
void OV2640_Init(void) {
    uint8_t value;
	
	  /*PWDN和RST配置*/
		Camera_PDRST_Init(); 
		HAL_GPIO_WritePin(CAM_PWDN_PORT,CAM_PWDN_PIN,0); 
		HAL_Delay(10);
	  HAL_GPIO_WritePin(CAM_RST_PORT,CAM_RST_PIN,0);
		HAL_Delay(10);
		HAL_GPIO_WritePin(CAM_RST_PORT,CAM_RST_PIN,1);
    // 读取设备 ID，确保设备正常连接
    if (OV2640_ReadReg(0x0A, &value) != HAL_OK) {
        // 设备无响应，处理错误
        Error_Handler();
    }
		printf("Product IDH is %x \n",value);
    // 检查返回的设备 ID
    if (value != 0x26) {
        // 如果设备 ID 不对，设备可能没有正确连接
        Error_Handler();
    }
		else{
			//printf("Product IDH is %x \n",value);
		}

    // 重置 OV2640 摄像头，复位所有寄存器
    if (OV2640_WriteReg(0x0C, 0x0C) != HAL_OK) {
        // 写入失败
        Error_Handler();
    }

    // 设置分辨率为 160x120（适用于 OV2640 的低分辨率模式）
    if (OV2640_WriteReg(0x12, 0x80) != HAL_OK) { // COM7: 设置到默认模式
        // 写入失败
        Error_Handler();
    }
    
    // 配置分辨率为 160x120
    if (OV2640_WriteReg(0x12, 0x40) != HAL_OK) {  // 设置 160x120 分辨率
        // 写入失败
        Error_Handler();
    }
    
    // 设置数据格式为 RAW 格式
    if (OV2640_WriteReg(0x15, 0x00) != HAL_OK) {  // COM8: 设置为 RAW 数据格式
        // 写入失败
        Error_Handler();
    }

    // 设置帧率（例如，10 fps）
    // 帧率控制寄存器设置，减少帧率，使其不太高
    // 设置的帧率与 PLL 时钟有关
    if (OV2640_WriteReg(0x11, 0x00) != HAL_OK) {  // 设置为较低的帧率
        // 写入失败
        Error_Handler();
    }

    // 进一步配置 HREF 和 VREF，以适应图像捕获
    if (OV2640_WriteReg(0x17, 0x16) != HAL_OK) {  // 设置水平同步信号
        // 写入失败
        Error_Handler();
    }
    
    if (OV2640_WriteReg(0x18, 0x04) != HAL_OK) {  // 设置垂直同步信号
        // 写入失败
        Error_Handler();
    }

    // 配置一些其他必要的寄存器以确保图像稳定
    if (OV2640_WriteReg(0x24, 0x40) != HAL_OK) {  // 设置自动增益控制（AGC）
        // 写入失败
        Error_Handler();
    }

    // 配置手动曝光
    if (OV2640_WriteReg(0x25, 0x18) != HAL_OK) {  // 手动曝光控制
        // 写入失败
        Error_Handler();
    }

    // 配置输出为 RGB565 格式（如果要 RAW 数据输出，可能需要其他配置）
    if (OV2640_WriteReg(0x26, 0x00) != HAL_OK) {  // 输出格式设置
        // 写入失败
        Error_Handler();
    }

    // 使能摄像头开始拍摄
    if (OV2640_WriteReg(0x12, 0x40) != HAL_OK) {
        // 启动摄像头
        Error_Handler();
    }

    // 测试读取帧（如果需要，可以在此处进行图像处理）
}

void OV2640_EnableAutoExposure(void) {
    // 开启 AEC, AGC, AWB
    if (OV2640_WriteReg(0x13, 0xE7) != HAL_OK) {  // COM8 寄存器
        Error_Handler();
    }

    // 自动曝光范围设置（你可以根据需求微调）
    OV2640_WriteReg(0x24, 0x40);  // AEW 上限
    OV2640_WriteReg(0x25, 0x30);  // AEB 下限
    OV2640_WriteReg(0x26, 0xA1);  // VPT 目标亮度值

    // 可选窗口控制
    OV2640_WriteReg(0x2D, 0x00);
    OV2640_WriteReg(0x2E, 0x00);
    OV2640_WriteReg(0x2F, 0x00);
}


void Start_Capture(void) {
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)frame_buffer, FRAME_WIDTH * FRAME_HEIGHT / 2);
}