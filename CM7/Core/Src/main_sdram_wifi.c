/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
	* @Authors				: Sheldon Liu & Weijie Chen
  ******************************************************************************
  * @attention V1.0 ~ V4.0 basic app
	* @attention V5.0, Insert app from STM32H747 DISCO BSP
	* 
	* @note: V1.0 GPIO and UART Test (2024.12.1) [Weijie Chen & Sheldon Liu]
	*				- USART1: For Debug Console	（dbg console）
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
  * @note: V5.0 CAMERA Test
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "quadspi.h"
#include "sai.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "sdram_fmc_drv.h"
//#include "microphone_sai_drv.h"
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

/*Wifi Module*/

// for future app 
uint8_t wifi_rxBuffer[WIFI_RX_BUFFER_SIZE];
uint8_t wifi_rdata;
uint8_t wifi_rflag;



/*Debug Console Module*/



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
  /* USER CODE BEGIN 2 */

/* @note: Key Issues PA0_C and PA1_C */
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);
	Tri_Color_LED_Init(); 
  Ext_SDRAM_Init();
	
	//USER_PDM_Filter_Init();
	
	HAL_UARTEx_ReceiveToIdle_IT(&huart4,wifi_rx_tmp_buf,WIFI_TMP_RXBUFF_SIZE);
	HAL_UARTEx_ReceiveToIdle_IT(&huart1,dbg_rx_tmp_buf,DBG_TMP_RXBUFF_SIZE);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
/***************************** Module Test***********************/	
	
/****************V1.0   GPIO 和 DBG USART ***********************/
	Tri_LED_On(LED_B); // 打开RGB的蓝色灯
	printf("Test Start!\n"); // 测试USART1,通过STLINK3的UART连接，同时测试重定向  
	
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
		* @note 测试是否带来系统的稳定性问题
		*/
	

/****************V4.0   Wifi Test ***********************/	
	/*采用配合DBG Console的方式测试wifi的功能*/
	/*
	* Step1: AT+HELP 查看帮助
	* Step2: AT+WSCAN 扫描WIFI特点名称
	* Step3: AT+WJAP=MMSysLab,1234509876
	* 
	*/
#ifdef DBG_WIFI_TEST
	
	//int32_t command_buff_size = 100;
	uint8_t at_command_buf[WIFI_RX_BUFFER_SIZE];
	memset(at_command_buf,0,WIFI_RX_BUFFER_SIZE); // 初始化AT命令缓冲区
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
	
/* Default forever loop */	
  while (1)
  {
    /* USER CODE END WHILE */
		
    /* USER CODE BEGIN 3 */
		HAL_Delay(1000);
		Tri_LED_On(LED_B);
		
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
	//

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
  RCC_OscInitStruct.PLL.PLLQ = 6;
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





///**
//  * @brief SAI Revc
//  * @retval None
//	* @Author Sheldon Liu
//	* @Date 2025.1.13
//  */

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
