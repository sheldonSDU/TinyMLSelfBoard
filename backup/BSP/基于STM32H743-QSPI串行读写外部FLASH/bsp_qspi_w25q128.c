/*
 * bsp_quadspi_W25Q128.c
 *
 *  Created on: Apr 25, 2021
 *      Author: Administrator
 *
 *
 *
 *
 *
 */
#include <bsp_qspi_w25q128.h>


/* 仅限本文件使用的函数 */
static void QSPI_W25Qx_Write_Enable(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_W25Qx_AutoPollingMemRead(uint32_t Timeout);
static void QSPI_W25Qx_Enter(QSPI_HandleTypeDef *hqspi);
static void QSPI_W25Qx_Exit(QSPI_HandleTypeDef *hqspi);

/**
  * 函数功能: 读取Flash状态并等待操作结束
  * 输入参数: Timeout：等待时间
 *
  * 返回值: Flash的状态
  * 说明:
 *
 *
 */
static uint8_t QSPI_W25Qx_AutoPollingMemRead(uint32_t Timeout)
{
	QSPI_CommandTypeDef     s_command={0};
    QSPI_AutoPollingTypeDef s_config={0};

    /* 基本配置 */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

    /* 配置自动轮询模式 */
    s_command.Instruction       = READ_STATUS_REG1_CMD;  //读取状态寄存器
    s_command.AddressMode       = QSPI_ADDRESS_NONE;     //没有地址
    s_command.DataMode          = QSPI_DATA_1_LINE;      //1线数据
    s_command.DummyCycles       = 0;                     //无空周期

    /* 配置自动轮询寄存器（不断查询状态寄存器bit0，等待其为0） */
    s_config.Match           = 0x00;				   //等待其为0
    s_config.Mask            = W25Q128FV_FSR_BUSY;     //状态寄存器bit0
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;	   //逻辑与
    s_config.StatusBytesSize = 1;
    s_config.Interval        = 0x10;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    /* 自动轮询模式等待编程结束 */
    if(HAL_QSPI_AutoPolling(&hqspi,&s_command,&s_config,Timeout) != HAL_OK)
    {
	   user_Assert(__FILE__,__LINE__);
    }

    return FLASH_OK;

}

/**
  * 函数功能: 读取外部FLASH的ID
  * 输入参数: 无
 *
  * 返回值: uint32_t
  * 说明:
 *  1、使用SPI模式的指令
 *
 */
uint32_t QSPI_W25Qx_ReadID(void)
{
	uint32_t uiID;

	QSPI_CommandTypeDef s_command = {0};
	uint8_t buf[3];

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 读取JEDEC ID */
	s_command.Instruction       = READ_JEDEC_ID_CMD;  //读取ID命令: 0x9F
	s_command.AddressMode       = QSPI_ADDRESS_NONE;  //没有地址
	s_command.DataMode          = QSPI_DATA_1_LINE;   //1线数据
	s_command.DummyCycles       = 0;                  //无空周期
	s_command.NbData            = 3;                  //读取三个数据

	/* 发送指令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 启动接收 */
	if(HAL_QSPI_Receive(&hqspi,buf,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	uiID = (buf[0] << 16) | (buf[1] << 8) | buf[2];

	return uiID;
}

/**
  * 函数功能:  外部FLASH写使能
  * 输入参数:  hqspi  QSPI_HandleTypeDef句柄
 *
  * 返回值: 无
  * 说明:
 *  1、使用SPI模式的指令
 *
 */
static void QSPI_W25Qx_Write_Enable(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command = {0};
	QSPI_AutoPollingTypeDef s_config={0};

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	//s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 写入使能配置 */
	s_command.Instruction       = WRITE_ENABLE_CMD;   //写使能命令
	s_command.AddressMode       = QSPI_ADDRESS_NONE;  //没有地址
	s_command.DataMode          = QSPI_DATA_NONE;     //没有数据
	s_command.DummyCycles       = 0;                  //无空周期
	s_command.NbData            = 0;                  //空数据

	/* 发送指令 */
	if(HAL_QSPI_Command(hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 配置自动轮询模式等待操作完成 */
	s_config.Match           = W25Q128FV_FSR_WREN;
	s_config.Mask            = W25Q128FV_FSR_WREN;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction = READ_STATUS_REG1_CMD;
	s_command.DataMode    = QSPI_DATA_1_LINE;
	s_command.NbData      = 1;

	if(HAL_QSPI_AutoPolling(hqspi,&s_command,&s_config,HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

}

/**
  * 函数功能:  擦除外部FLASH的扇区（大小：4KB)
  * 输入参数:  _uiSectorAddr: 扇区地址，以4KB为单位的地址，比如0，4096，8192等
 *
  * 返回值: 无
  * 说明:
 *  1、
 *
 */
void QSPI_W25Qx_EraseSector(uint32_t _SectorAddr)
{
	QSPI_CommandTypeDef	s_command = {0};

	/* 写使能 */
	QSPI_W25Qx_Write_Enable(&hqspi);

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 擦除扇区配置 */
	s_command.Instruction       = SECTOR_ERASE_CMD;     //扇区擦除指令
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;  //1线地址方式
	s_command.DataMode          = QSPI_DATA_NONE;       //没有数据
	s_command.Address           = _SectorAddr;          //扇区的首地址，保证是4KB整数倍
	s_command.DummyCycles       = 0;                    //无空周期

	/* 发送指令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 自动轮询模式等待编程结束 */
	if(QSPI_W25Qx_AutoPollingMemRead(W25Q128FV_SUBSECTOR_ERASE_MAX_TIME) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

}

/**
  * 函数功能:  页编程，通过QSPI将数据写入外部FALSH
  * 输入参数:  _pBuf: 需要存入数据的指针
 *          _write_Addr: 目标区域首地址，即页首地址，比如0，256，512等。
 *          _write_Size: 数据个数，不能超过页的大小，可以填入（1 ~ 256）
  * 返回值: 无
  * 说明:
 *    1、华邦的W25Q128FV仅仅支持SPI模式写入
 */
uint8_t QSPI_W25Qx_Write_Buffer(uint8_t *_pBuf,uint32_t _write_Addr,uint16_t _write_Size)
{
	QSPI_CommandTypeDef	s_command = {0};

	/* 防止写入的大小超过256字节 */
	if(_write_Size > W25Q128FV_PAGE_SIZE)
	{
		/* 进入断言，提示错误 */
		user_Assert(__FILE__,__LINE__);
	}

	QSPI_W25Qx_Write_Enable(&hqspi);  //写使能

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/*写入序列配置 */
	s_command.Instruction       = QUAD_INPUT_PAGE_PROG_CMD;   //24位四线快速写入指令
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;        //1线地址方式
	s_command.DataMode          = QSPI_DATA_4_LINES;          //4线数据方式
	s_command.Address           = _write_Addr;                //写入数据的地址
	s_command.NbData            = _write_Size;                //写入数据的大小
	s_command.DummyCycles       = 0;                          //无空周期

	/* 发送指令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 启动传输 */
	if(HAL_QSPI_Transmit(&hqspi,_pBuf,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 自动轮询模式等待编程结束 */
	if(QSPI_W25Qx_AutoPollingMemRead(W25Q128FV_SUBSECTOR_ERASE_MAX_TIME) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	return 1;
}

/**
  * 函数功能:  外部FLASH芯片进入QSPI模式
  * 输入参数:  *hqspi: qspi句柄
 *
  * 返回值: 无
  * 说明:
 *
 */
static void QSPI_W25Qx_Enter(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef	s_command = {0};
	/* 配置FLASH进入QPSI模式 */
	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 写入序列配置 */
	s_command.Instruction       = ENTER_QPI_MODE_CMD;         //进入QSPI模式
	s_command.AddressMode       = QSPI_ADDRESS_NONE;          //无地址方式
	s_command.DataMode          = QSPI_DATA_NONE;             //无数据方式
	s_command.DummyCycles       = 0;                          //0空闲状态周期

	/* 发送指令 */
	if(HAL_QSPI_Command(hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

}

/**
  * 函数功能:  外部FLASH芯片退出QSPI模式
  * 输入参数:  *hqspi: qspi句柄
 *
  * 返回值: 无
  * 说明:
 *
 */
static void QSPI_W25Qx_Exit(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef	s_command = {0};

    /* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;	  //4线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 写入序列配置 */
	s_command.Instruction       = EXIT_QPI_MODE_CMD;         //进入QSPI模式
	s_command.AddressMode       = QSPI_ADDRESS_NONE;          //无地址方式
	s_command.DataMode          = QSPI_DATA_NONE;             //无数据方式
	s_command.DummyCycles       = 0;                          //0空闲状态周期

	/* 发送指令 */
	if(HAL_QSPI_Command(hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}
}

/**
  * 函数功能:  连续读取若干字节，字节的个数不能超出芯片容量
  * 输入参数:  _pBuf: 读取数据的存放地址
 *          _read_Addr: 起始的地址
 *          _read_Size: 数据个数，可以大于W25Q128FV_PAGE_SIZE,但不能超出芯片总容量
  * 返回值: 无
  * 说明:
 *    1、从SPI模式切换到QSPI模式，读取完毕后切换回SPI模式（其他函数仅仅支持SPI模式）。
 */
void QSPI_W25Qx_Read_Buffer(uint8_t *_pBuf,uint32_t _read_Addr,uint32_t _read_Size)
{
	QSPI_CommandTypeDef	s_command = {0};

	/* 进入QSPI模式 */
	QSPI_W25Qx_Enter(&hqspi);

	/* 开始从FLASH读取数据 */
	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/*读取序列配置 */
	s_command.Instruction       = QUAD_INOUT_FAST_READ_CMD;     //24位四线快速写入指令
	s_command.AddressMode       = QSPI_ADDRESS_4_LINES;       //4线地址方式
	s_command.DataMode          = QSPI_DATA_4_LINES;          //4线数据方式
	s_command.Address           = _read_Addr;                 //写入数据的地址
	s_command.NbData            = _read_Size;                 //写入数据的大小
	s_command.DummyCycles       = 2;                          //两个空闲状态周期（4个时钟周期），结合时序理解

	/* 发送指令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 读取数据 */
	if(HAL_QSPI_Receive(&hqspi,_pBuf,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 退出QSPI模式 */
	QSPI_W25Qx_Exit(&hqspi);

}

/**
  * 函数功能: 复位外部Flash
  * 输入参数: 无
 *
  * 返回值: void
  * 说明:
 *
 */
void QSPI_W25Qx_Reset_Memory()
{
	QSPI_CommandTypeDef s_command = {0};

	/* 基本配置 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;	  //1线方式发送指令
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;       //24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  //无交替字节
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      //W25Q128FV不支持DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  //DDR模式，数据输出延迟
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   //每次传输都发指令

	/* 复位使能W25x */
	s_command.Instruction       = RESET_ENABLE_CMD;   //复位使能命令
	s_command.AddressMode       = QSPI_ADDRESS_NONE;  //没有地址
	s_command.DataMode          = QSPI_DATA_NONE;     //没有数据
	s_command.DummyCycles       = 0;                  //无空周期

	/* 发送复位使能命令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 发送复位命令 */
	s_command.Instruction       = RESET_MEMORY_CMD;   //复位命令
	/* 发送复位使能命令 */
	if(HAL_QSPI_Command(&hqspi,&s_command,HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

	/* 自动轮询模式等待等待完成 */
	if(QSPI_W25Qx_AutoPollingMemRead(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		user_Assert(__FILE__,__LINE__);
	}

}

