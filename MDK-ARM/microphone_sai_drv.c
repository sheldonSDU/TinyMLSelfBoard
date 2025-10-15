/**
  ******************************************************************************
  * @file    micropyhon_sai_drv.c
  * @brief   This file provides code for the utilization
  *          of Microphone MP34DT06JTR
  ******************************************************************************
  * @attention.
  *
  ******************************************************************************
  */

#include "microphone_sai_drv.h"
#define PDM_BUFFER_SIZE 256  // 定义 PDM 缓冲区大小
uint8_t pdm_buffer[PDM_BUFFER_SIZE] __attribute__ ((section(".ARM.__at_0x24000000")));  // PDM 数据缓冲区

#define PCM_BUFFER_SIZE 256  // PCM 缓冲区大小
int16_t pcm_buffer[PCM_BUFFER_SIZE] __attribute__ ((section(".ARM.__at_0x24010000")));

PDM_Filter_Handler_t PDM_FilterHandler;
PDM_Filter_Config_t PDM_FilterConfig;


void Start_PDM_Receive(void)
{
    // 使用 DMA 接收 PDM 数据
    if (HAL_SAI_Receive_DMA(&hsai_BlockA1, pdm_buffer, PDM_BUFFER_SIZE) != HAL_OK)
    {
        Error_Handler();
    }
		
}

void USER_PDM_Filter_Init(void)
{
    PDM_FilterHandler.bit_order = PDM_FILTER_BIT_ORDER_LSB;   // PDM 数据位顺序
    PDM_FilterHandler.endianness = PDM_FILTER_ENDIANNESS_LE;  // 小端模式
    PDM_FilterHandler.high_pass_tap = 2122358088;             // 高通滤波器系数
    PDM_FilterHandler.out_ptr_channels = 1;                  // 输出通道数
    PDM_FilterHandler.in_ptr_channels = 1;                   // 输入通道数
    PDM_Filter_Init(&PDM_FilterHandler);

    PDM_FilterConfig.output_samples_number = PCM_BUFFER_SIZE;  // 输出 PCM 样本数
    PDM_FilterConfig.mic_gain = 24;                           // 增益
    PDM_FilterConfig.decimation_factor = PDM_FILTER_DEC_FACTOR_64;  // 降采样因子
    PDM_Filter_setConfig(&PDM_FilterHandler, &PDM_FilterConfig);
}


void Process_PDM_To_PCM(void)
{
    
    PDM_Filter(pdm_buffer, pcm_buffer, &PDM_FilterHandler);
   
}