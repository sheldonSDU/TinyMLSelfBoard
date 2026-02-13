/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for STM32H747 development board
  * @authors        : Sheldon Liu & Weijie Chen
  * @version        : V11.0
  * @date           : 2025-10
  ******************************************************************************
  * @attention 
  * - V1.0~V4.0: Basic peripheral testing phase
  * - V5.0: Integrated STM32H747 DISCO BSP applications
  * - V6.0: Camera driver development with AI assistance
  * - Note: Always verify serial port tool compatibility, Attention!
  * 
  * @copyright      : [Add copyright information if applicable]
  ******************************************************************************
  * Version History:
  * 
  * V1.0 (2024-12-01) - GPIO & UART Test
  *   - USART1: Debug console with printf redefinition (completed 2025-01-05)
  *   - USART3: Raspberry Pi Pico communication (requires TX/RX cross-connection)
  *   - Implemented IT + IDLE + Double Ping Pong buffer for USART1
  * 
  * V2.0 (2025-01-06~2025-01-08) - SDRAM Test
  *   - Address: 0xC0000000, Capacity: 32MB
  *   - Configuration: 100MHz clock, CAS latency 3
  *   - Test coverage: Word/Byte read/write operations
  *   - Reference: https://blog.csdn.net/qq_45467083/article/details/109425825
  *   - Note: MPU restrictions may interfere with SDRAM debugging operations
  * 
  * V3.0 (2025-01-09) - Power Management Test
  *   - Configurable regulator voltages: 1.0V/1.1V/1.2V via __HAL_PWR_VOLTAGESCALING_CONFIG()
  *   - Successfully verified INA3221 I2C communication with Raspberry Pi Pico
  * 
  * V4.0 - WIFI Module Test
  *   - Command format: Terminate with "\r\n" (0x0d, 0x0a)
  *   - Socket communication functionality validated
  *   - Critical: PA0_C/PA1_C pin configuration required for UART4
  *   - Reference: https://community.st.com/t5/stm32-mcus-products/subject-stm32h753zi-cannot-control-pc02-as-gpio-or-spi2-miso/td-p/655439
  * 
  * V5.0 - QSPI Flash Test
  *   - Basic operations: ID read, chip erase, page read/write verified
  *   - Requirement: GPIO pins must be configured for very high speed
  *   - Future plan: Implement quad SPI mode
  *   - Reference: https://www.waveshare.net/study/article-658-1.html
  * 
  * V6.0 - Camera Interface Test
  *   - Device ID verified: 0x5640
  *   - Reference: https://blog.csdn.net/qq_40500005/article/details/112055533
  * 
  * V7.0 - SAI PDM Microphone Test
  *   - [Add specific implementation details when available]
  * 
  * V8.0 - (2025-08-01) Memory Performance Analysis  
  *   - Tested read latency of DTCM, SRAM, and SDRAM
  *   - Implemented cycle-accurate measurement using DWT counter
  * 
  * V9.0 - CMSIS-NN Integration
  *   - Separated neural network implementations into nn.h/nn.c files
  *   - Together with x-cube-ai in the future!
  * 
  * V10.0 - (2026-01-01) STM32-PICO Communication Framework
  *   - Implemented high-speed UART communication with minimal protocol overhead
  *   - Added cyclic test data transmission mechanism
  * 
  * V11.0 - (2026-02-01)MobileNet V1 Inference Test
  *   - Optimized 27-layer CNN implementation using Weijie Chen library
  *   - Achieved real-time inference performance on STM32H747
  *   - Added detailed performance metrics and result visualization
	*
	* V12.0 -  Accelarator Component Enable 
	*
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/*  */
#include "sdram_fmc_drv.h"
#include "qspi_flash_drv.h"
#include "microphone_sai_drv.h"
#include "ov5640_dcmi_drv.h"
#include "stm32_pico_uart_drv.h"
// 新增模型推理依赖头文件
#include "add_h/arm_math.h"
#include "add_h/genModel.h"
#include "add_h/image_data.h"
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
#define DBG_EXSDRAM_TEST 0
#define DBG_WIFI_TEST 0
#define DBG_QSPIFLASH_TEST 0
#define DBG_CAMERADCMI_TEST 1
#define DBG_MICROPHONE_TEST 0
#define DBG_MEMORY_LATENCY_TEST 0  /* 内存延迟测试开关 */


#define DBG_MODEL_INFERENCE_TEST 1  /* 神经网络模型推理测试开关：1开启，0关闭 */

#define DBG_STM32_TO_PICO_TEST 1  /*STM32 TO PICO 1=启用发送，0=禁用发送 */


/* 内存测试参数 */
#define TEST_ITERATIONS 1000000    /* 测试迭代次数 */
#define BUFFER_SIZE     1024       /* 测试缓冲区大小 */

/*mobilenet v1 test*/
#define DBG_MODEL_INFERENCE_TEST 1  /* 神经网络模型推理测试开关：1开启，0关闭 */
/*STM32 TO PICO*/
#define DBG_STM32_TO_PICO_TEST 1  // 1=启用发送，0=禁用发送

void invoke_layers(void) ;
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


/* Refer to Cmix-nn */

#define USR_CC_RESET() \
  do { \
    __asm__ volatile("" ::: "memory"); \
    *DWT_CYCCNT = 0; \
    __asm__ volatile("" ::: "memory"); \
  } while (0)


#define USR_GET_CC_TIMESTAMP(x) \
  do { \
    __asm__ volatile("" ::: "memory"); \
    x = (*(volatile unsigned int *) DWT_CYCCNT); \
    __asm__ volatile("" ::: "memory"); \
  } while (0)


	

	

/* USER CODE END 0 */

/**
  * @brief  The appllication entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  int32_t timeout;
	int32_t ret;
	SCB_EnableICache();  // 启用指令缓存 I-Cache
	SCB_EnableDCache();  // 启用数据缓存 D-Cache
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
//timeout = 0xFFFF;
//while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
//if ( timeout < 0 )
//{
//Error_Handler();
//}
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
	
/* @note: Key Issues PA0_C and PA1_C , for uart3*/
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);
	Tri_Color_LED_Init(); 
  Ext_SDRAM_Init();
	
	//USER_PDM_Filter_Init();
	
	HAL_UARTEx_ReceiveToIdle_IT(&huart4,wifi_rx_tmp_buf,WIFI_TMP_RXBUFF_SIZE);
	HAL_UARTEx_ReceiveToIdle_IT(&huart1,dbg_rx_tmp_buf,DBG_TMP_RXBUFF_SIZE);
	
	
	
	/***************************** Module Test***********************/	
	
/****************V1.0   GPIO AND DBG USART ***********************/
	Tri_LED_On(LED_B); // 打开RGB的蓝色灯
  
	printf("TinyML PLatform Start!\n"); // 测试USART1,通过STLINK3的UART连接，同时测试重定向  
	
	/****************Memory Latency Test***********************/
#if DBG_MEMORY_LATENCY_TEST
	InitDWT();
	TestMemoryLatency();
#endif
/****************V2.0   SDRAM Test ***********************/
#if DBG_EXSDRAM_TEST	
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
	//假设已经初始化了buffer数组
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
	* Step3: AT+WJAP=MMSysLab,123459876//MIMIMI,208208208. 连接热点
	* Step4: AT+SOCKET=4,192.168.123.105,1234 建立Socket 通信,获取conID
	* Step5: AT+SOCKETSENDLINE=conID,10,1234567890 , 发?10字节的shu?
	* Step6: AT+SOCKETREAD=conID  读取接收的数?
	* 
	*/
#if DBG_WIFI_TEST
	
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

#if DBG_QSPIFLASH_TEST
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


/****************V7.0   CAMERA Test ***********************/
/*
*  (1)Camera Config Interface : 
*  (2)Camera Data Interface:   
*/
	 
#if DBG_CAMERADCMI_TEST	
	 /*2025.3.18 Cameraid ok!*/
	 printf("------------Start Camera Test ------------------");
	 ret = CAM_Init(CAM_R160x120,CAM_PF_RGB565);
	// printf("CAMERA ID is %u",
//Start_Capture();
#endif //DBG_CAMERADCMI_TEST
	 
	 
/****************V8.0   MICROPHONE Test ***********************/

#if  DBG_MICROPHONE_TEST 
	 Start_PDM_Receive();
	 HAL_Delay(10);
	 Process_PDM_To_PCM();
#endif	 // DBG_MICROPHONE_TEST
	 
	 
	 
/****************V9.0 CMSIS-NN ***********************/
// 原CMSIS-NN注释保留
/****************V10.0 STM32 TO PICO ***********************/
  /****************STM32到PICO通信任务***********************/
#ifdef DBG_STM32_TO_PICO_TEST
  stm32_to_pico_communication_task();
#endif
/****************V11.0  Neural Network Model Inference Test***********************/
#if DBG_MODEL_INFERENCE_TEST
	//HAL_Delay(500);

    uint32_t start, end;
    printf("\n=== Neural Network Model Inference Test ===\n");
    
    // 记录推理开始时间
		start = HAL_GetTick();
		printf("Start Time: %d ms\n", start);
    //start = HAL_GetTick();
    

    // 调用模型推理核心函数
    invoke_layers();
    
    // 记录推理结束时间
    end = HAL_GetTick();
    printf("End Time: %d ms\n", end);

    // 打印推理耗时
    printf("Inference Time: %d ms\n", end - start);
    
    // 打印模型输出结果（打印前100个元素，按10个/行格式化）
    printf("Model Output (first 100 elements):\n");
    for (int i = 0; i < 100; i++) {
        printf("%-5d", buffer1[i]);
        if ((i + 1) % 10 == 0) {
            printf("\n");  // 每10个元素换行，提升可读性
        }
    }
    printf("\n=== Model Inference Test Complete ===\n");
#endif // DBG_MODEL_INFERENCE_TEST
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		printf(" --- Platform is running ---- \r\n");
		HAL_Delay(1000);
		Tri_LED_On(LED_B);
		HAL_Delay(1000);
		Tri_LED_Off(LED_B);
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
  * @brief  神经网络模型推理核心函数
  * @note   包含27层卷积与深度可分离卷积运算，基于CMSIS-NN优化实现
  */
arm_status
arm_convolve_HWC_u8_u4_u4_icn(const uint8_t *Im_in,
                    const uint16_t dim_im_in,
                    const uint16_t ch_im_in,
                    const uint8_t *wt,
                    const uint16_t ch_im_out,
                    const uint16_t dim_kernel,
                    const uint8_t left_padding,
                    const uint8_t right_padding,
                    const uint8_t top_padding,
                    const uint8_t bottom_padding,
                    const uint16_t stride,
                    const int32_t *bias,
                    uint8_t *Im_out,
                    const uint16_t dim_im_out,
                    const uint8_t z_in,
                    const uint8_t z_wt,
                    const uint8_t z_out,
                    const int32_t *m_zero,
                    const int8_t *n_zero,
                    int16_t * bufferA,
                    uint8_t *bufferB);
arm_status
arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(const uint8_t * Im_in,
                        const uint16_t dim_im_in,
                        const uint16_t ch_im_in,
                        const uint8_t * wt,
                        const uint16_t ch_im_out,
                        const uint16_t dim_kernel,
                        const uint8_t left_padding,
                        const uint8_t right_padding,
                        const uint8_t top_padding,
                        const uint8_t bottom_padding,
                        const uint16_t stride,
                        const int32_t * bias,
                        uint8_t * Im_out,
                        const uint16_t dim_im_out,
                        const uint8_t z_in,
                        const uint8_t z_wt,
                        const uint8_t z_out,
                        const int32_t *m_zero,
                        const int8_t *n_zero,
                        int16_t * bufferA,
                        uint8_t * bufferB);
arm_status
arm_convolve_HWC_u4_u4_u4_icn(const uint8_t *Im_in,
                    const uint16_t dim_im_in,
                    const uint16_t ch_im_in,
                    const uint8_t *wt,
                    const uint16_t ch_im_out,
                    const uint16_t dim_kernel,
                    const uint8_t left_padding,
                    const uint8_t right_padding,
                    const uint8_t top_padding,
                    const uint8_t bottom_padding,
                    const uint16_t stride,
                    const int32_t *bias,
                    uint8_t *Im_out,
                    const uint16_t dim_im_out,
                    const uint8_t z_in,
                    const uint8_t z_wt,
                    const uint8_t z_out,
                    const int32_t *m_zero,
                    const int8_t *n_zero,
                    int16_t * bufferA,
                    uint8_t *bufferB);
void invoke_layers(void) {
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	//HAL_Delay(500);
  // Layer 0 convolution
  arm_convolve_HWC_u8_u4_u4_icn( 
		  input_image, 160, 4, WEIGHT_0, 8, 3, 1, 1, 1, 1, 2, BIAS_0, buffer1, 80, 0, Z_W_0, 0, M_ZERO_0, N_ZERO_0, bufferA, bufferB);
  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  // Layer 1 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 80, 8, WEIGHT_1, 8, 3, 1, 1, 1, 1, 1, BIAS_1, buffer0, 80, 0, Z_W_1, 0, M_ZERO_1, N_ZERO_1, bufferA, bufferB);
//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
   // Layer 2 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 80, 8, WEIGHT_2, 16, 1, 0, 0, 0, 0, 1, BIAS_2, buffer1, 80, 0, Z_W_2, 0, M_ZERO_2, N_ZERO_2, bufferA, bufferB);
//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
   // Layer 3 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 80, 16, WEIGHT_3, 16, 3, 1, 1, 1, 1, 2, BIAS_3, buffer0, 40, 0, Z_W_3, 0, M_ZERO_3, N_ZERO_3, bufferA, bufferB);
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
   // Layer 4 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 40, 16, WEIGHT_4, 32, 1, 0, 0, 0, 0, 1, BIAS_4, buffer1, 40, 0, Z_W_4, 0, M_ZERO_4, N_ZERO_4, bufferA, bufferB);
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
   // Layer 5 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 40, 32, WEIGHT_5, 32, 3, 1, 1, 1, 1, 1, BIAS_5, buffer0, 40, 0, Z_W_5, 0, M_ZERO_5, N_ZERO_5, bufferA, bufferB);

   // Layer 6 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 40, 32, WEIGHT_6, 32, 1, 0, 0, 0, 0, 1, BIAS_6, buffer1, 40, 0, Z_W_6, 0, M_ZERO_6, N_ZERO_6, bufferA, bufferB);

   // Layer 7 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 40, 32, WEIGHT_7, 32, 3, 1, 1, 1, 1, 2, BIAS_7, buffer0, 20, 0, Z_W_7, 0, M_ZERO_7, N_ZERO_7, bufferA, bufferB);

   // Layer 8 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 20, 32, WEIGHT_8, 64, 1, 0, 0, 0, 0, 1, BIAS_8, buffer1, 20, 0, Z_W_8, 0, M_ZERO_8, N_ZERO_8, bufferA, bufferB);

   // Layer 9 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 20, 64, WEIGHT_9, 64, 3, 1, 1, 1, 1, 1, BIAS_9, buffer0, 20, 0, Z_W_9, 0, M_ZERO_9, N_ZERO_9, bufferA, bufferB);

   // Layer 10 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 20, 64, WEIGHT_10, 64, 1, 0, 0, 0, 0, 1, BIAS_10, buffer1, 20, 0, Z_W_10, 0, M_ZERO_10, N_ZERO_10, bufferA, bufferB);

   // Layer 11 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 20, 64, WEIGHT_11, 64, 3, 1, 1, 1, 1, 2, BIAS_11, buffer0, 10, 0, Z_W_11, 0, M_ZERO_11, N_ZERO_11, bufferA, bufferB);

   // Layer 12 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 64, WEIGHT_12, 128, 1, 0, 0, 0, 0, 1, BIAS_12, buffer1, 10, 0, Z_W_12, 0, M_ZERO_12, N_ZERO_12, bufferA, bufferB);

   // Layer 13 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_13, 128, 3, 1, 1, 1, 1, 1, BIAS_13, buffer0, 10, 0, Z_W_13, 0, M_ZERO_13, N_ZERO_13, bufferA, bufferB);

   // Layer 14 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 128, WEIGHT_14, 128, 1, 0, 0, 0, 0, 1, BIAS_14, buffer1, 10, 0, Z_W_14, 0, M_ZERO_14, N_ZERO_14, bufferA, bufferB);

   // Layer 15 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_15, 128, 3, 1, 1, 1, 1, 1, BIAS_15, buffer0, 10, 0, Z_W_15, 0, M_ZERO_15, N_ZERO_15, bufferA, bufferB);

   // Layer 16 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 128, WEIGHT_16, 128, 1, 0, 0, 0, 0, 1, BIAS_16, buffer1, 10, 0, Z_W_16, 0, M_ZERO_16, N_ZERO_16, bufferA, bufferB);

   // Layer 17 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_17, 128, 3, 1, 1, 1, 1, 1, BIAS_17, buffer0, 10, 0, Z_W_17, 0, M_ZERO_17, N_ZERO_17, bufferA, bufferB);

   // Layer 18 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 128, WEIGHT_18, 128, 1, 0, 0, 0, 0, 1, BIAS_18, buffer1, 10, 0, Z_W_18, 0, M_ZERO_18, N_ZERO_18, bufferA, bufferB);

   // Layer 19 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_19, 128, 3, 1, 1, 1, 1, 1, BIAS_19, buffer0, 10, 0, Z_W_19, 0, M_ZERO_19, N_ZERO_19, bufferA, bufferB);

   // Layer 20 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 128, WEIGHT_20, 128, 1, 0, 0, 0, 0, 1, BIAS_20, buffer1, 10, 0, Z_W_20, 0, M_ZERO_20, N_ZERO_20, bufferA, bufferB);

   // Layer 21 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_21, 128, 3, 1, 1, 1, 1, 1, BIAS_21, buffer0, 10, 0, Z_W_21, 0, M_ZERO_21, N_ZERO_21, bufferA, bufferB);

   // Layer 22 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 10, 128, WEIGHT_22, 128, 1, 0, 0, 0, 0, 1, BIAS_22, buffer1, 10, 0, Z_W_22, 0, M_ZERO_22, N_ZERO_22, bufferA, bufferB);

   // Layer 23 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 10, 128, WEIGHT_23, 128, 3, 1, 1, 1, 1, 2, BIAS_23, buffer0, 5, 0, Z_W_23, 0, M_ZERO_23, N_ZERO_23, bufferA, bufferB);

   // Layer 24 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 5, 128, WEIGHT_24, 256, 1, 0, 0, 0, 0, 1, BIAS_24, buffer1, 5, 0, Z_W_24, 0, M_ZERO_24, N_ZERO_24, bufferA, bufferB);

   // Layer 25 depthwise convolution
   arm_depthwise_separable_conv_HWC_u4_u4_u4_icn(
     buffer1, 5, 256, WEIGHT_25, 256, 3, 1, 1, 1, 1, 1, BIAS_25, buffer0, 5, 0, Z_W_25, 0, M_ZERO_25, N_ZERO_25, bufferA, bufferB);

   // Layer 26 convolution
   arm_convolve_HWC_u4_u4_u4_icn(
     buffer0, 5, 256, WEIGHT_26, 256, 1, 0, 0, 0, 0, 1, BIAS_26, buffer1, 5, 0, Z_W_26, 0, M_ZERO_26, N_ZERO_26, bufferA, bufferB);
}

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
