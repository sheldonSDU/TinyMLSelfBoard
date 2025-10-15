/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
	* @Authors				: Sheldon Liu & Weijie Chen
  ******************************************************************************
  * @attention V1.0 ~ V4.0 basic app
	* @attention V5.0, Insert app from STM32H747 DISCO BSP
	* @attention V6.0, Insert Cam Code with  Chatgpt
	* @attention Don't forget the error of serial port tools!
	*
	*
	* @note: V1.0 GPIO and UART Test (2024.12.1) [Weijie Chen & Sheldon Liu]
	*				- USART1: For Debug Console	(dbg console?)
	*         - printf redefine OK![Sheldon Liu  2025.1.5]
	*					- 
	*				- USART3: Connecting to Rasp Pico, - Swap Tx & Rx
	*       - IT + IDLE + DOUBLE Ping Pong with USART1
	*	@note: V2.0 SDRAM Test	(2025.1.6~2025.1.8) [Sheldon Liu]
	*			  - ADDRESS 0xC000_0000
	*				- Capacity: 32MB
	*       - Clock:100Mhz, CAS:3
	*       - Test Pattern: Word R/W &Byte R/W
	*				- refer to : https://blog.csdn.net/qq_45467083/article/details/109425825
  * @attention: MPU stops the writing to SDRAM which is started by the app!
  *             For memory debugger, MPU make no sense.
	*				
	* @note: V3.0 Power Test
	*        - internal regulator output voltage : 1.0V 1.1V 1.2V 
	*          __HAL_PWR_VOLTAGESCALING_CONFIG(),Modify the value
  *        - INA3221 IIC Test Passed with Raspberry Pico (2025.1.9) [Sheldon Liu]
  *				 
	*	@note: V4.0 WIFI Test 
	*        - End of Command: "\r" 0x0d, "\n" 0x0a
	*				 - Test with Socket Connection
	*				 - !UART4 TX&RX pin PA0_C PA1_C should be 
	*				 - refer to : https://community.st.com/t5/stm32-mcus-products/subject-stm32h753zi-cannot-control-pc02-as-gpio-or-spi2-miso/td-p/655439
	*	
  * @note: V5.0 QPSI Test
	*        - Read ID OK
	*        - GPIO should be very high speed
	*        - Read &Write & Erase OK
	*        - Quad Test in Future!
	*        - refer to https://www.waveshare.net/study/article-658-1.html
	* 
	* @note: V6.0 Camera Test
	*        - Read ID ： ADDR_H:1C  ADDR_L:1D
	*        - refer to https://blog.csdn.net/qq_40500005/article/details/112055533
	*
	*
	* @note: V7.0 SAI PDM Microphone Test
	*
	* @note: V8.0 Memory Performance Test 
	*        - Test DTCM, SRAM, SDRAM read latency
  *        - 
  * @note: V9.0 CMSIS-NN 
	*					- To distinguish between neural network and interface test cases, the neural network tests are conducted separately in the files nn.h and nn.c
	*					- examples in nn.c
	*         - 
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "quadspi.h"
#include "sai.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"
#include "stm32h7xx_hal_dcmi.h"

#include "arm_nnfunctions.h"
#include "arm_math.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "sdram_fmc_drv.h"
#include "qspi_flash_drv.h"
#include "microphone_sai_drv.h"
#include "ov2640_dcmi_drv.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/*Module Test Switch*/
#define DBG_EXSDRAM_TEST 1
#define DBG_WIFI_TEST 1
#define DBG_QSPIFLASH_TEST 1
//#define DBG_CAMERADCMI_TEST 1
//#define DBG_MICROPHONE_TEST 1
#define DBG_MEMORY_LATENCY_TEST 1  /* 内存延迟测试开关 */

/* 内存测试参数 */
#define TEST_ITERATIONS 1000000    /* 测试迭代次数 */
#define BUFFER_SIZE     1024       /* 测试缓冲区大小 */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/*每个module的宏定义在各自的.h文件*/
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/*SDRAM Module*/
uint32_t bsp_TestExtSDRAM(void);

/*Memory Latency Test Module*/
void InitDWT(void);
uint32_t GetDWT_CycleCount(void);
void TestMemoryLatency(void);

/*Wifi Module*/

// for future app 
uint8_t wifi_rxBuffer[WIFI_RX_BUFFER_SIZE];
uint8_t wifi_rdata;
uint8_t wifi_rflag;



/*Debug Console Module*/
uint8_t dbg_hello[] = "Hello,TinyML";


/*camera module */
uint8_t frame_ready = 0;


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*Redefine the function of printf(), USART1 is used for log*/
__ASM(".global __use_no_semihosting\n\t");               
struct FILE  { int handle; };     // 标准库需要的支持函数
FILE __stdout;                           // FILE 在stdio.h文件
void _sys_exit(int x) {x = x; }         // 定义_sys_exit()以避免使用半主机模式

int fputc(int ch, FILE *f)               // 重定向fputc函数，使printf的输出，由fputc输出到UART,  这里使用串口1(USART1)
{   
    //if(xFlag.PrintfOK == 0) return 0;  // 判断USART是否已配置，防止在配置前调用printf被卡
       
    while(( USART1->ISR&0X40)==0);        // 等待上一次串口数据发送完  
    USART1 ->TDR = (uint8_t) ch;                   // 写DR,串口1将发 
    return ch;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  int32_t timeout;

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
/**************************  Initlization  ********************************/
/* USER CODE END Boot_Mode_Sequence_0 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
//  timeout = 0xFFFF;
//  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
//  if ( timeout < 0 )
//  {
//  Error_Handler();
//  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_FMC_Init();
  MX_QUADSPI_Init();
  MX_SAI1_Init();
  MX_UART4_Init();
  MX_DCMI_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

/* @note: Key Issues PA0_C and PA1_C */
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);
	Tri_Color_LED_Init(); 
  Ext_SDRAM_Init();
	
	//USER_PDM_Filter_Init();
	
	//HAL_UARTEx_ReceiveToIdle_IT(&huart4,wifi_rx_tmp_buf,WIFI_TMP_RXBUFF_SIZE);
	//HAL_UARTEx_ReceiveToIdle_IT(&huart1,dbg_rx_tmp_buf,DBG_TMP_RXBUFF_SIZE);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
/***************************** Module Test***********************/	
	
/****************V1.0   GPIO AND DBG USART ***********************/
	Tri_LED_On(LED_B); // 打开RGB的蓝色灯
	printf("Test Start!\n"); // 测试USART1,通过STLINK3的UART连接，同时测试重定向  
	
	/****************Memory Latency Test***********************/
#ifdef DBG_MEMORY_LATENCY_TEST
	InitDWT();
	TestMemoryLatency();
#endif
/****************V2.0   SDRAM Test ***********************/
#ifdef DBG_EXSDRAM_TEST	
	/*Single Word and Single Byte Test*/
	uint8_t write_byte_data = 0x55;
	uint32_t write_word_data = 0x12345678;
	*(volatile uint8_t*)EXT_SDRAM_ADDR = write_byte_data;
	*(volatile uint32_t*)(EXT_SDRAM_ADDR+0x1000) = write_word_data;
	uint32_t read_word_data = *(volatile uint32_t*)(EXT_SDRAM_ADDR+0x1000);
	uint8_t read_byte_data = *(volatile uint8_t*)EXT_SDRAM_ADDR;
	
	if(read_byte_data == write_byte_data 
		&& (read_word_data == write_word_data)) // 数据读取正确
	{
    printf("SDRAM Single Test Passed !\n");
	}
	else
	{
		 printf("SDRAM Single Test Failed !\n");
	}
		
	//uint32_t buffer[EXT_SDRAM_SIZE];
	// 假设已经初始化了buffer数组
	for(int i = 0; i < EXT_SDRAM_SIZE; i++)
	{
    *(volatile uint8_t*)(EXT_SDRAM_ADDR + i) = i;
	}
	for(int j = 0; j < EXT_SDRAM_SIZE; j++)
	{
    if(*(volatile uint8_t*)(EXT_SDRAM_ADDR + j)!= j)
    {
        // 数据读取错误
        break;
				printf("SDRAM Multi Test Failed !\n");
    }
	}
	for(int i = 0; i < EXT_SDRAM_SIZE/4; i++)
	{
    *((volatile uint32_t*)(EXT_SDRAM_ADDR) + i) = i;
	}
	for(int j = 0; j < EXT_SDRAM_SIZE/4; j++)
	{
    if(*((volatile uint32_t*)(EXT_SDRAM_ADDR) + j)!= j)
    {
        // 数据读取错误
        break;
				printf("SDRAM Multi Test Failed !\n");
    }
	}
	
	printf("SDRAM Multi Test Passed !\n");
#endif // DBG_EXSDRAM_TEST	
	
	
/****************V3.0   Power Test ***********************/	
	 //__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	 /*
		* @note 测试是否带来系统的稳定?
		*/
	

/****************V4.0   Wifi Test ***********************/	
	/*采用配合DBG Console的方式测试wifi的功能?*/
	/*
	* Refer to : https://aithinker-combo-guide.readthedocs.io/en/latest/docs/command-set/Wi-Fi_AT_Commands.html#at-wjap-ap
	* Step1: AT+HELP 查看帮助
	* Step2: AT+WSCAN 扫描WIFI特点名称
	* Step3: AT+WJAP=MIMIMI,208208208. 连接热点
	* Step4: AT+SOCKET=4,192.168.123.105,1234 建立Socket 通信,获取conID
	* Step5: AT+SOCKETSENDLINE=conID,10,1234567890 , 发?10字节的shu?
	* Step6: AT+SOCKETREAD=conID  读取接收的数?
	* 
	*/
#ifdef DBG_WIFI_TEST
	
	//int32_t command_buff_size = 100;
	uint8_t at_command_buf[WIFI_RX_BUFFER_SIZE];
	memset(at_command_buf,0,WIFI_RX_BUFFER_SIZE); // 初始化AT命令缓冲�?
	printf("Start Wifi Test, help with Dbg console! \n");
	while(1) 
	{
		if(dbg_rx_size !=0 && (dbg_rx_size < (WIFI_RX_BUFFER_SIZE - 2))) // 防止溢出
		{
			//当dbg console接收到命令，复制到at_command_buf,并补充\r\n
			memcpy(at_command_buf,dbg_rx_final_buf,dbg_rx_size);
			at_command_buf[dbg_rx_size] = '\r';
			at_command_buf[dbg_rx_size] = '\n';
			HAL_UART_Transmit(&huart4,at_command_buf,(dbg_rx_size+2),HAL_MAX_DELAY);
			dbg_rx_size = 0;
		}
		HAL_Delay(20);
		
	}
#endif 	
	
/****************V5.0   QSPI Test ***********************/	

#ifdef DBG_QSPIFLASH_TEST
	uint8_t  qspistatus; //
	//qspistatus = W25Q256JVFIQ_ReadStatus();
	W25Q256JVFIQ_EraseSector(0x100000); //擦写测试
	
	// 示例：向地址0x000000写入数据
   uint8_t flash_write_data[] = "Hello, hengheng!";
   W25Q256JVFIQ_PageWrite(0x100000, flash_write_data, sizeof(flash_write_data));
   
	 // 示例：从地址0x000000读取数据
   uint8_t flash_read_data[sizeof(flash_write_data)];
   W25Q256JVFIQ_Read(0x100000, flash_read_data, sizeof(flash_read_data));
	 for(int i = 0; i< sizeof(flash_write_data); i++)
	 {
			if(flash_write_data[i] != flash_read_data[i] )
			{
				printf("QSPI Flash Test Failed!\n");
				break;
			}
			
	 }
	 printf("QSPI Flash Test OK!\n");
	 // Quad IO Test 
//	 uint8_t flash_write_data_4lines[] = "Hello, Hengheng!";
//	 W25Q256JVFIQ_PageWrite4Lines(0x20000,flash_write_data_4lines,sizeof(flash_write_data_4lines));
//	 uint8_t flash_read_data_4lines[sizeof(flash_write_data_4lines)];
//	 W25Q256JVFIQ_Read4Lines(0x20000,flash_read_data_4lines,sizeof(flash_read_data_4lines));
	 
	 
	 
#endif // DBG_QSPIFLASH_TEST	


	 
	 
#ifdef DBG_CAMERADCMI_TEST	
	 /*2025.3.18 Cameraid ok!*/
	 OV2640_Init();
	 OV2640_EnableAutoExposure(); 
	 //Start_Capture();
#endif //DBG_CAMERADCMI_TEST
	 
	 
/****************V6.0   MICROPHONE Test ***********************/

#ifdef  DBG_MICROPHONE_TEST 
	 Start_PDM_Receive();
	 HAL_Delay(10);
	 Process_PDM_To_PCM();
#endif	 // DBG_MICROPHONE_TEST
	 
	 
	 
/* Default forever loop */	
  while (1)
  {
    /* USER CODE END WHILE */
//		if (__HAL_DCMI_GET_FLAG(&hdcmi, DCMI_FLAG_FRAMERI)) {
//            /* 清除帧完成标志 */
//            __HAL_DCMI_CLEAR_FLAG(&hdcmi, DCMI_FLAG_FRAMERI);
//            
//            /* 通过USART发送帧数据 */
//            HAL_UART_Transmit(&huart1, (uint8_t*)frame_buffer, FRAME_SIZE, 1000);
//      }
//    /* USER CODE BEGIN 3 */
//		//Start_PDM_Receive();

		
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  初始化DWT寄存器用于精确计时，由TRAE IDE协助生成
  * @retval None
	* @date 2025.10
  */
void InitDWT(void)
{
    /* 使能DWT外设 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    /* 重置计数器 */
    DWT->CYCCNT = 0;
    /* 使能计数器 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  获取当前DWT计数器值
  * @retval 当前CPU周期计数
  */
uint32_t GetDWT_CycleCount(void)
{
    return DWT->CYCCNT;
}

/**
  * @brief  测试DTCM SRAM和SDRAM的读取延迟
  * @retval None
  * @note: 测试DTCM SRAM和SDRAM的读取延迟，分别在DTCM和SDRAM中分配内存，测试读取延迟
    -在DTCM中分配内存 (DTCM通常在0x20000000地址开始) 
    -在SDRAM中分配内存 (根据代码中的SDRAM测试，SDRAM地址为EXT_SDRAM_ADDR)
  * @date 2025.10
  */
volatile uint32_t dtcm_buffer[BUFFER_SIZE] __attribute__((section(".ARM.__at_0x20000000")));
volatile uint32_t sram_buffer[BUFFER_SIZE] __attribute__((section(".ARM.__at_0x24030000")));
volatile uint32_t *sdram_buffer = (volatile uint32_t *)EXT_SDRAM_ADDR;
void TestMemoryLatency(void)
{
    /* 在DTCM中分配内存 (DTCM通常在0x20000000地址开始) */    
    /* 在SDRAM中分配内存 (根据代码中的SDRAM测试，SDRAM地址为EXT_SDRAM_ADDR) */
    
    uint32_t start_cycle, end_cycle;
    uint64_t total_dtcm_cycles = 0;
    uint64_t total_sram_cycles = 0;
    uint64_t total_sdram_cycles = 0;
    uint32_t dummy = 0;  // 用于避免编译器优化掉读取操作
    
    printf("\n=== Memory Latency Test Start ===\n");
    
    /* 初始化缓冲区数据 */
    printf("Initializing buffers...\n");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        dtcm_buffer[i] = i;
        sram_buffer[i] = i;
        sdram_buffer[i] = i;
    }
    
    /* 预热缓存 */
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        dummy += dtcm_buffer[i];
        dummy += sram_buffer[i];
        dummy += sdram_buffer[i];
    }
    
    /* 测试DTCM读取延迟 */
    printf("Testing DTCM read latency...\n");
    for (int iter = 0; iter < TEST_ITERATIONS; iter++)
    {
        int index = iter % BUFFER_SIZE;
        start_cycle = GetDWT_CycleCount();
        dummy = dtcm_buffer[index];  // 读取DTCM中的数据
        end_cycle = GetDWT_CycleCount();
        total_dtcm_cycles += (end_cycle - start_cycle);
    }
    /* 测试SRAM读取延迟 */
    printf("Testing SRAM read latency...");
        for (int iter = 0; iter < TEST_ITERATIONS; iter++)
    {
        int index = iter % BUFFER_SIZE;
        start_cycle = GetDWT_CycleCount();
        dummy = sram_buffer[index];  // 读取SRAM中的数据
        end_cycle = GetDWT_CycleCount();
        total_sram_cycles += (end_cycle - start_cycle);
    }


    /* 测试SDRAM读取延迟 */
    printf("Testing SDRAM read latency...\n");
    for (int iter = 0; iter < TEST_ITERATIONS; iter++)
    {
        int index = iter % BUFFER_SIZE;
        start_cycle = GetDWT_CycleCount();
        dummy = sdram_buffer[index];  // 读取SDRAM中的数据
        end_cycle = GetDWT_CycleCount();
        total_sdram_cycles += (end_cycle - start_cycle);
    }
    
    /* 计算平均延迟 */
    double avg_dtcm_cycles = (double)total_dtcm_cycles / TEST_ITERATIONS;
    double avg_sram_cycles = (double)total_sram_cycles / TEST_ITERATIONS;
    double avg_sdram_cycles = (double)total_sdram_cycles / TEST_ITERATIONS;
    
    /* 获取系统时钟频率 */
    double sys_clk_mhz = (double)SystemCoreClock / 1000000.0;
    double ns_per_cycle = 1000.0 / sys_clk_mhz;
    
    /* 输出结果 */
    printf("\n=== Memory Latency Test Results ===\n");
    printf("System Clock: %.2f MHz\n", sys_clk_mhz);
    printf("Time per CPU cycle: %.2f ns\n", ns_per_cycle);
    printf("\nDTCM SRAM:");
    printf("\n  Average read latency: %.2f cycles (%.2f ns)\n", avg_dtcm_cycles, avg_dtcm_cycles * ns_per_cycle);
    printf("SRAM:");
    printf("\n  Average read latency: %.2f cycles (%.2f ns)\n", avg_sram_cycles, avg_sram_cycles * ns_per_cycle);
    printf("SDRAM:");
    printf("\n  Average read latency: %.2f cycles (%.2f ns)\n", avg_sdram_cycles, avg_sdram_cycles * ns_per_cycle);
    printf("\nSDRAM latency is %.2f times higher than DTCM\n", avg_sdram_cycles / avg_dtcm_cycles);
    printf("=====================================\n\n");
    
    /* 使用dummy变量防止编译器优化 */
    if (dummy == 0) {}
}

//void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
//{
//    if (hsai->Instance == SAI1_Block_A)
//    {
//        Process_PDM_To_PCM();  // 调用解码函数
//    }
//		//Start_PDM_Receive();
//}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
