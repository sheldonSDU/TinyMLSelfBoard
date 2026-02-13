#ifndef __STM32_PICO_UART_DRV_H__
#define __STM32_PICO_UART_DRV_H__

#include <stdio.h>
#include "usart.h"
void stm32_to_pico_communication_task(void);
HAL_StatusTypeDef stm32_send_to_pico(uint8_t *data, uint8_t data_len);
#endif
