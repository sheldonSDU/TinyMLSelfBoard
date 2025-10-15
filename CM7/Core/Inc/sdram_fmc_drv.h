/**
 *@file    sdram_fmc_drv.h
 *@brief   使用 FMC 操作 SDRAM
 *@note    此驱动测试 W9825G6KH SDRAM芯片通过
*/

#ifndef _SDRAM_FMC_DRV_H_
#define _SDRAM_FMC_DRV_H_

#include "fmc.h"

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



/*Defination for external sdram*/
#define EXT_SDRAM_ADDR  	((uint32_t)0xC0000000)
#define EXT_SDRAM_SIZE		(8 * 1024 * 1024)

void Ext_SDRAM_Init(void);
uint32_t TestExtSDRAM(void);
#endif /* _SDRAM_FMC_DRV_H_ */