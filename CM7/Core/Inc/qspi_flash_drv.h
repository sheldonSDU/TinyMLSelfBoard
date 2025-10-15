/**
 *@file    qspi_flash_drv.h
 *@brief   使用 QSPI接口测试Flash芯片
 *@note    此驱动测试 W25Q SDRAM芯片通过
*/

#ifndef _QSPI_FLASH_DRV_H_
#define _QSPI_FLASH_DRV_H_

#include "quadspi.h"

#define HAL_QPSI_TIMEOUT_DEFAULT 10000


#define W25Q256_CMD_READ_ID           0x9F
#define W25Q256_CMD_READ_DATA         0x03
#define W25Q256_CMD_FAST_READ         0x0B
#define W25Q256_CMD_WRITE_ENABLE      0x06
#define W25Q256_CMD_PAGE_PROGRAM      0x02
#define W25Q256_CMD_SECTOR_ERASE      0x20
#define W25Q256_CMD_BULK_ERASE        0xC7
#define W25Q256_CMD_STATUS_REGISTER   0x05
#define W25Q256_CMD_WRITE_STATUS_REG  0x01  // status1
#define W25Q256_CMD_QUAD_READ_DATA    0x6B
#define W25Q256_CMD_QUAD_PAGE_PROGRAM 0x32
#define W25Q256_CMD_WRITE_STATUS2_REG 0x31
void W25Q256JVFIQ_PageWrite(uint32_t address, 
		uint8_t *data, uint32_t size);

void W25Q256JVFIQ_Read(uint32_t address, 
	uint8_t *data, uint32_t size);

void W25Q256JVFIQ_EraseSector(uint32_t address);

uint16_t W25Q256JVFIQ_ReadID(void);


void W25Q256JVFIQ_Read4Lines(uint32_t address, uint8_t *data, uint32_t size);
void W25Q256JVFIQ_PageWrite4Lines(uint32_t address, 
		uint8_t *data, uint32_t size);


uint8_t W25Q256JVFIQ_ReadStatus(void);
#endif /* _QSPI_FLASH_DRV_H_ */