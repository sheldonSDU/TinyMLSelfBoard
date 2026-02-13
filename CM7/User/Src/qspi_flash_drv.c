#include "qspi_flash_drv.h"
#include <stdio.h>

static void QSPI_SendCommand(QSPI_CommandTypeDef *cmd)
{
	 if (HAL_QSPI_Command(&hqspi, cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }
}

// 等待W25Q256JVFIQ处于就绪状态
static void W25Q256JVFIQ_WaitForReady(void)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_STATUS_REGISTER; // 读状态寄存器1
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = 1;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    uint8_t status;
    do
    {
        QSPI_SendCommand(&cmd);
        HAL_QSPI_Receive(&hqspi, &status, HAL_QPSI_TIMEOUT_DEFAULT);
    } while ((status & 0x01) == 0x01); // 等待BUSY位为0
}

// 写使能函数
static void W25Q256JVFIQ_WriteEnable(void)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_WRITE_ENABLE; // 写使能命令
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.NbData = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    QSPI_SendCommand(&cmd);
}





// 页写函数
void W25Q256JVFIQ_PageWrite(uint32_t address, uint8_t *data, uint32_t size)
{
    W25Q256JVFIQ_WriteEnable();
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_PAGE_PROGRAM; // 页写命令
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = size;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.Address = address;

    QSPI_SendCommand(&cmd);
    HAL_QSPI_Transmit(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT);
    W25Q256JVFIQ_WaitForReady();
}

// 读函数
void W25Q256JVFIQ_Read(uint32_t address, uint8_t *data, uint32_t size)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_READ_DATA; // 读数据命令
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = size;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.Address = address;

    QSPI_SendCommand(&cmd);
    HAL_QSPI_Receive(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT);
}

// 擦除扇区函数
void W25Q256JVFIQ_EraseSector(uint32_t address)
{
    W25Q256JVFIQ_WriteEnable();
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_SECTOR_ERASE; // 扇区擦除命令
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.NbData = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.Address = address;
    QSPI_SendCommand(&cmd);
    W25Q256JVFIQ_WaitForReady();
}

uint16_t W25Q256JVFIQ_ReadID(void)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_READ_ID; // 读状态寄存器1
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = 2;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    uint8_t pData[2];
    QSPI_SendCommand(&cmd);
    HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT);
		printf("QSPIFlash ID is %xh,%xh \n",pData[1],pData[0]);
		return pData[1] | ( pData[0] << 8);
}


//void W25Q256JVFIQ_Enable4Line(void)
//{
//    QSPI_CommandTypeDef cmd;
//    cmd.Instruction = W25Q256_CMD_WRITE_ENABLE; // 写使能命令
//    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
//    cmd.AddressMode = QSPI_ADDRESS_NONE;
//    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//    cmd.DataMode = QSPI_DATA_NONE;
//    cmd.DummyCycles = 0;
//    cmd.NbData = 0;
//    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
//    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
//    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
//    QSPI_SendCommand(&cmd);
//}


uint8_t W25Q256JVFIQ_ReadStatus(void)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_WRITE_STATUS2_REG; // 写使能命令
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.NbData = 1;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    QSPI_SendCommand(&cmd);
	  uint8_t status;
		HAL_QSPI_Receive(&hqspi, &status, HAL_QPSI_TIMEOUT_DEFAULT);
		return status;
}


// quad 页写函数
void W25Q256JVFIQ_PageWrite4Lines(uint32_t address, uint8_t *data, uint32_t size)
{
    W25Q256JVFIQ_WriteEnable();
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_QUAD_PAGE_PROGRAM; // 页写命令
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 0;
    cmd.NbData = size;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.Address = address;

    QSPI_SendCommand(&cmd);
    HAL_QSPI_Transmit(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT);
    W25Q256JVFIQ_WaitForReady();
}

// quad 读函数
void W25Q256JVFIQ_Read4Lines(uint32_t address, uint8_t *data, uint32_t size)
{
    QSPI_CommandTypeDef cmd;
    cmd.Instruction = W25Q256_CMD_QUAD_READ_DATA; // 读数据命令
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 0;
    cmd.NbData = size;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.Address = address;

    QSPI_SendCommand(&cmd);
    HAL_QSPI_Receive(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT);
}