/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sai.h
  * @brief   This file contains all the function prototypes for
  *          the sai.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __SAI_H__
#define __SAI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stm32h7xx.h"
/* USER CODE END Includes */

extern SAI_HandleTypeDef hsai_BlockA1;

/* USER CODE BEGIN Private defines */
//#define PDM_CLOCK_FREQ      2048000U   // PDM Clock = 2.048MHz
//#define PDM_DECIMATION      64         // Decimation factor (32kHz output)
//#define PDM_BUFFER_SIZE     512        // DMA buffer size
//#define PCM_BUFFER_SIZE     (PDM_BUFFER_SIZE/8)  // PCM buffer size after decimation
/* USER CODE END Private defines */

void MX_SAI1_Init(void);

/* USER CODE BEGIN Prototypes */
/* Function Prototypes */
//HAL_StatusTypeDef MP34DT06_PDM_Init(void);
//HAL_StatusTypeDef MP34DT06_PDM_Start(uint16_t* pBufferPDM, uint16_t size);
//HAL_StatusTypeDef MP34DT06_PDM_Stop(void);
//void MP34DT06_PDM_FilterConfig(void);
//void MP34DT06_PDM_ProcessData(uint16_t* pBufferPDM, int16_t* pBufferPCM, uint16_t size);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SAI_H__ */

