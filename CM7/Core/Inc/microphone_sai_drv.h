/**
 *@file    sdram_fmc_drv.h
 *@brief   使用 FMC 操作 SDRAM
 *@note    此驱动测试 W9825G6KH SDRAM芯片通过
*/

#ifndef _MICROPHONE_SAI_DRV_H_
#define _MICROPHONE_SAI_DRV_H_

#include "sai.h"

//#include "arm_math.h"
#include "pdm2pcm_glo.h"  // STM32 提供的 PDM 转 PCM 支持	


void Start_PDM_Receive(void);
void Process_PDM_To_PCM(void);
void USER_PDM_Filter_Init(void);
#endif /* _MICROPHONE_SAI_DRV_H_ */