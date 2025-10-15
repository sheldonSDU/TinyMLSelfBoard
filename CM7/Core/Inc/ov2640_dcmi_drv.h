/**
 *@file    ov2640_dcmi_drv.h
 *@brief   使用i2c接口控制ov2640，并通过dcmi读取摄像头数据
 *@note    2025.3.10
*/

#ifndef _OV2640_DCMI_DRV_H_
#define _OV2640_DCMI_DRV_H_

#include "main.h"
#include "dcmi.h"
#include "usart.h"
#include "i2c.h"

#define OV2640_ID_HIGH 0x1c
#define OV2640_ID_LOW  0x1d
//#define OV2640_

/* OV2640 I2C 地址 */
#define OV2640_I2C_ADDRESS 0x60  // 设备地址（7位地址左移1位）
#define FRAME_WIDTH  160
#define FRAME_HEIGHT 120
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT )


extern uint8_t frame_buffer[FRAME_SIZE] __attribute__((aligned(32)));

HAL_StatusTypeDef OV2640_WriteReg(uint8_t reg, uint8_t value);
HAL_StatusTypeDef OV2640_ReadReg(uint8_t reg, uint8_t *value);

void OV2640_Init(void);
void OV2640_EnableAutoExposure(void);
void Start_Capture(void);

#endif /* _OV2640_DCMI_DRV_H_ */