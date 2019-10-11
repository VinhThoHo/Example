
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "lcd1602.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "ds18b20_mflib.h"
#include "Timeh.h"
#include "stm32f1xx_hal_rtc.h"

RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart1;
uint32_t time_us;
static uint32_t temp=0;
char sprintfTemp[20];
int dem=0;
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
	uint8_t Date;
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;	


#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
PUTCHAR_PROTOTYPE 
{
 HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
 return ch;
}

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void RTC_CalendarShow();

	void TIM1_UP_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim1);
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_11);
}

uint32_t return_time;
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
	
	hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	HAL_RTC_Init(&hrtc);
		if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2) //0x32F2
		{
			return_time = set_time(2018, 9, 21, 10, 13, 30);    //Setup Time
		
	  RTC_WriteTimeCounter(&hrtc,return_time) ;  // ghi vao thanh ghi gia tri Timestamp
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);   
		}
		else
		{
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
      printf("\n ..");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
      printf("\n ...");
    }
    /* Clear source Reset Flag */
    __HAL_RCC_CLEAR_RESET_FLAGS();
	  }
	MX_TIM1_Init();
	LCD_Init();
	 HAL_TIM_Base_Start_IT(&htim1);
	 float temp;

	HAL_RTCEx_SetSecond_IT(&hrtc);
		
  while (1) 
  {
    RTC_CalendarShow();
		ds18b20_init_seq();
		ds18b20_send_rom_cmd(SKIP_ROM_CMD_BYTE);
		ds18b20_send_function_cmd(CONVERT_T_CMD);
		HAL_Delay(2);
		ds18b20_init_seq();
		ds18b20_send_rom_cmd(SKIP_ROM_CMD_BYTE);
		ds18b20_send_function_cmd(READ_SCRATCHPAD_CMD);
		temp = ds18b20_read_temp();	// returns float value
		sprintf((char *)sprintfTemp, "%0.1f",temp);
		LCD_Gotoxy(10,0);
		LCD_Puts(sprintfTemp);
		HAL_Delay(100);
	}

}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

static void MX_RTC_Init(void)
{
	/* Vi su dung thu vien Timeh.h nen chung ta khong can su dung ham setup thoi gian nay.*/
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;
	
		sTime.Hours = 0x16;
		sTime.Minutes = 0x28;
		sTime.Seconds = 0x39;
		 
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
		printf("________________Setup_Time______________");

		DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
		DateToUpdate.Month = RTC_MONTH_JANUARY;
		DateToUpdate.Date = 0x01;
		DateToUpdate.Year = 0x00;
		HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD);
		printf("________________Setup_Date______________");
			
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);

}
static void RTC_CalendarShow()
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
	char sprintfTime[20];
	char sprintfDate[20];
	uint32_t time = RTC_ReadTimeCounter(&hrtc);
	struct tm time_struct =convert_time_stamp(time);

  sprintf((char *)sprintfTime, "%0.2d:%0.2d:%0.2d", time_struct.tm_hour,time_struct.tm_min,time_struct.tm_sec);
  sprintf((char *)sprintfDate, ">>>%0.2d-%0.2d-%0.2d<<<",time_struct.tm_mday,time_struct.tm_mon, 2000 + time_struct.tm_year-100);
	
	printf("\nTimestamp = %d\n", time);
	printf("tm.Second  = %d\n",time_struct.tm_sec);
	printf("tm.Minute  = %d\n",time_struct.tm_min);
	printf("tm.Hour    = %d\n",time_struct.tm_hour);
	printf("tm.Wday    = %d\n",time_struct.tm_wday);  // chu y cac thu trong tuan (0-6)
	printf("tm.Day     = %d\n",time_struct.tm_mday);
	printf("tm.Month   = %d\n",time_struct.tm_mon);
	printf("tm.Year    = %d\n",time_struct.tm_year-100);
	printf("\n-----------------------------------\n");
	
		LCD_Gotoxy(0,0);
		LCD_Puts(sprintfTime);
		LCD_Gotoxy(14,0);
		LCD_PutChar(223);
		LCD_Gotoxy(15,0);
		LCD_Puts("C");
		LCD_Gotoxy(0,1);
		LCD_Puts(sprintfDate);
		
}

static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7199;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();        
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC4 PC5 PC6 PC7 
                           PC8 PC9 PC10 */ //DATA LCD16x2 + Pin11 TIM1_EXTI
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;   //DATA DS18B20
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}
void _Error_Handler(char *file, int line)
{
  while(1)
  {
  }
}

