/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#define DBG_FINAL_RXBUFF_SIZE 256
#define DBG_TMP_RXBUFF_SIZE 128

#define WIFI_RX_BUFFER_SIZE 256
#define WIFI_TMP_RXBUFF_SIZE 64
/* USER CODE END Includes */

extern UART_HandleTypeDef huart4;

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
extern uint8_t dbg_rx_tmp_buf[DBG_TMP_RXBUFF_SIZE];     // 调试串口数据临时缓冲?
extern uint8_t dbg_rx_final_buf[DBG_FINAL_RXBUFF_SIZE]; 


extern uint8_t wifi_rx_tmp_buf[DBG_TMP_RXBUFF_SIZE];     // 调试串口数据临时缓冲?
extern uint8_t wifi_rx_final_buf[DBG_FINAL_RXBUFF_SIZE]; 


extern uint8_t dbg_rx_size;
/* USER CODE END Private defines */

void MX_UART4_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

