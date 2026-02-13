/**
  ******************************************************************************
  * @file    stm32_pico_uart_drv.c
  * @brief   This file provides code for stm32 communication with 
                    raspberry pico based on uart
  ******************************************************************************
  * @attention.
  *
  ******************************************************************************
  */

  #include "stm32_pico_uart_drv.h"
 


   HAL_StatusTypeDef stm32_send_to_pico(uint8_t *data, uint8_t data_len) {
    // 直接发送原始数据，跳过所有帧头/尾、CRC校验等封装逻辑
    return HAL_UART_Transmit(&huart3, data, data_len, HAL_MAX_DELAY);
}

/**
 * @brief STM32到PICO通信任务函数
 * @note 每秒发送一次测试数据到PICO，可通过DBG_STM32_TO_PICO_TEST开关控制是否执行
 */
void stm32_to_pico_communication_task(void) {

        char test_data[32] = {0};
        sprintf(test_data, "STM32 2 PICO TEST");
        
        HAL_StatusTypeDef ret = stm32_send_to_pico((uint8_t*)test_data, strlen(test_data));
        if (ret == HAL_OK) {
            printf("Send to Pico: %s\n", test_data);
        } else {
            printf("Send to Pico Failed!\n");
        }
    
}	