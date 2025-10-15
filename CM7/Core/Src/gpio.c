/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
/* Tri-color LED Module Initilization 
@
	
*/
void Tri_Color_LED_Init(){
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOK_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/**USART1 GPIO Configuration
    PK2    ------> LED_G
    PG3    ------> LED_R
	  PG14   ------> LED_B
    */
    GPIO_InitStruct.Pin = LED_G_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_G_PORT, &GPIO_InitStruct);
	
	
    GPIO_InitStruct.Pin = LED_B_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_B_PORT, &GPIO_InitStruct);
	
		GPIO_InitStruct.Pin = LED_R_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_R_PORT, &GPIO_InitStruct);
		
		
		/*Turn off all LEDS*/
		
	HAL_GPIO_WritePin(LED_B_PORT,LED_B_PIN,LED_OFF);
	HAL_GPIO_WritePin(LED_G_PORT,LED_G_PIN,LED_OFF);
	HAL_GPIO_WritePin(LED_R_PORT,LED_R_PIN,LED_OFF);
}
	


/* Camera PowerDown and Reset Initilization 
@
*/

void Camera_PDRST_Init(){
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /*CAM_PWDN --> PB1*/
    GPIO_InitStruct.Pin = CAM_PWDN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CAM_PWDN_PORT, &GPIO_InitStruct);
	
		 /*CAM_PWDN --> PB1*/
    GPIO_InitStruct.Pin = CAM_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CAM_RST_PORT, &GPIO_InitStruct);
	

		/*Turn off all LEDS*/	
	HAL_GPIO_WritePin(LED_B_PORT,LED_B_PIN,LED_OFF);
	HAL_GPIO_WritePin(LED_G_PORT,LED_G_PIN,LED_OFF);
}
	


/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

void Tri_LED_On(LEDs Color){
	if( Color == 0)
		HAL_GPIO_WritePin(LED_B_PORT,LED_B_PIN,LED_ON);
	if( Color == 1)
		HAL_GPIO_WritePin(LED_G_PORT,LED_G_PIN,LED_ON);
	if( Color == 2)
		HAL_GPIO_WritePin(LED_R_PORT,LED_R_PIN,LED_ON);
	
}

void Tri_LED_Off(LEDs Color){
	if( Color == 0)
		HAL_GPIO_WritePin(LED_B_PORT,LED_B_PIN,LED_OFF);
	if( Color == 1)
		HAL_GPIO_WritePin(LED_G_PORT,LED_G_PIN,LED_OFF);
	if( Color == 2)
		HAL_GPIO_WritePin(LED_R_PORT,LED_R_PIN,LED_OFF);
	
}

/* USER CODE END 1 */

/** Configure pins
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
*/
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
