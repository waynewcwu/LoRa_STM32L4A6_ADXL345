/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
STM32L4A6 ADXL345 loRa node
Version: V2
Update Note:
1.Update Calculate algorithm.
2.Update Data Acquire follow.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//TODO--Wayne20200924
PWRST_t PWRST;
USART_LoRa USARTLoRa;

uint8_t TrigAcqFlag = false;
uint8_t DRDY=0;
//int16_t sampleCount = 0;

uint8_t data[6]={0,0,0,0,0,0};
int16_t sampleIndex = 0;

char NodeStatus;

//TODO:for signal buffer

fftInstance XfftInstance;
fftInstance YfftInstance;
fftInstance ZfftInstance;
//TODO for IWDG Freeze By Standby Mode
FLASH_OBProgramInitTypeDef pOBInit;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
//TODO--wayne

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//TODO--wayne
/*void ADXL345_Init();

void AcquireData();
void FeatureExtraction();*/

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_IWDG_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  //TODO--Wayne20200925
  RUN_PWR_Mode_Init(&hrtc);
  USARTLoRa.Status=WakeUp;
//  USARTLoRa.Status=CMDdebug;
  while(HAL_UART_Receive_IT(&huart3,&USARTLoRa.Rbuffer,1)!=HAL_OK);
  //delay_init(168);
  //delay_init(80);//STM32L4A6 HCLK=80MHz
  delay_init(16);//STM32L4A6 HCLK=16MHz
  ADXL345_Init();

  IWDGFreezeBySTDBY();
  //HAL_NVIC_SystemReset();
  /*
  setting Device power mode:
  PWRST.PowerMode =
  			 	 ---RunMode
  			 	 ---StandbyMode
  */
  PWRST.PowerMode = StandbyMode;
  NodeStatus = TrigAcquire;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	//TODO:ADXL345 Measure
//     while(sampleIndex < dataLength/2)
//     {
//     	ADXL_IntProto();
//     	readRegister(DATA0,data,6);
//     	if(dataReady == true)
// 		{
// 			delay_us(5);
// 			 AcquireData(&data);
// 
// 			sampleIndex++;
// 			dataReady = false;
// 		}
//     }
//     FeatureExtraction();
//     sampleIndex = 0;
	  switch(NodeStatus)
	  {
	  	case TrigAcquire:
	  		TrigAcqFlag = true;
	  		NodeStatus = DataAcquire;
//	  		//reset Data Ready Interrupt, Start acquire data.
//	  		readRegister(DATA0,data,6);
	  		  break;

	  	case DataAcquire:
	  		if(TrigAcqFlag == true)
	  		{
	  			TrigAcqFlag = false;
	  			//reset Data Ready Interrupt, Start acquire data.
	  			readRegister(DATA0,data,6);
	  		}
	  		//if(sampleIndex < dataLength/2)
		  	while(sampleIndex < dataLength/2)
		  	{
		  		if(DRDY=1)
		  		{
			  		//Acquire data
			  		AcquireData(&data);
			  		sampleIndex++;
			  		DRDY=0;
			  	}
		  	}
			//else if(sampleIndex == dataLength/2)
	  		if(sampleIndex == dataLength/2)
	  			NodeStatus = DataCalculate;
	  		  break;

	  	case DataCalculate:
	  		FeatureExtraction();
    		sampleIndex = 0;
	  		NodeStatus = TransmitWithPWRST;
	  		  break;

	  	case TransmitWithPWRST:
	  		if(PWRST.PowerMode == StandbyMode)
			{
    			//TODO:ADXL345 Standby Entry
    			ADXL_Standby(ON);

    			//TODO:Lora send data
    			LoRa_USART(&huart3);

    			//IWDG_Refresh
    			HAL_IWDG_Refresh(&hiwdg);

    			//TODO:MCU Standby Entry
    			EnterStandbyPWR_Mode(&hrtc);
    		}

    		else
    		{
    			USARTLoRa.ResStatus = LoRa_OK;
    			//TODO:Lora send data
    			LoRa_USART(&huart3);

    			NodeStatus = TrigAcquire;

    			//IWDG_Refresh
    			HAL_IWDG_Refresh(&hiwdg);

    		}
	  		  break;
	  }





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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART3;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4000;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1600-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
//TODO-Wayne0200924
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//   if(GPIO_Pin == GPIO_PIN_13)//trigger start pin PC13
//   {
// 	  delay_ms(10);
// 	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==GPIO_PIN_SET)
// 	  {
// 		  if(!PWRST.WLUP_BTstatus )
// 			  PWRST.WLUP_BTstatus = 1;
// 	  }
//   }
  if(GPIO_Pin == GPIO_PIN_2)//SPI data ready
	{
	  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2)==GPIO_PIN_SET)
	  {
	  	DRDY=1;
//		  if(sampleIndex < dataLength/2)
//		  {
//			  //Acquire data
//			  AcquireData(&data);
//			  sampleIndex++;
//		  }

	  }

    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart3);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file
   */

  USARTLoRa.RevStrCount++; //all string receive count
  if(USARTLoRa.Rbuffer==0x0A)//when receive 0x0a("\n" Line feeds string),it is a effective command
  {
	  USARTLoRa.RevStrEndCount++;//0x0a("\n" Line feeds string) count
	  if( USARTLoRa.RevStrEndCount==1	&&	USARTLoRa.RevStrCount > 1)
		  USARTLoRa.ResetRevflag = 1;
  }

  if(!USARTLoRa.ResetRevflag)//if LoRa not reset
  {
  	  USARTLoRa.RevData[USARTLoRa.RxCount++]= USARTLoRa.Rbuffer;
  	  if(USARTLoRa.RevStrEndCount == 2)
  	  {
  		  USARTLoRa.Revflag = 1;
  		  USARTLoRa.RxCount= 0;
  		  USARTLoRa.RevStrEndCount = 0;
  		  USARTLoRa.RevStrCount = 0;
  		  char c;
  		  char RxHead_Check[] = "\n\r>> mac rx";
  		  for(c=0;c<=10;c++)
  		  {
  			USARTLoRa.RxHead[c]=USARTLoRa.RevData[c];
  		  }
  		  if(strcmp(USARTLoRa.RxHead, RxHead_Check ) == 0)
  			USARTLoRa.LoRaRxflag = 1;
  	  }
    }
    else
    {
    	if(USARTLoRa.RevStrEndCount > 6)
    	{
    		USARTLoRa.RevData[USARTLoRa.RxCount++]= USARTLoRa.Rbuffer;
    		if(USARTLoRa.RevStrEndCount == 9)
    		{
    			USARTLoRa.Revflag = 1;
    			USARTLoRa.RxCount= 0;
    			USARTLoRa.RevStrEndCount = 0;
    			USARTLoRa.RevStrCount = 0;
    		}
    	}

    }

  if(USARTLoRa.RxCount>=70)//if don't receive "\n" Line feeds string full over to 64 byte, clear data
  {
	  USARTLoRa.RxCount=0;
  	  memset( USARTLoRa.RevData, 0, strlen((const char*)USARTLoRa.RevData) ); //clear Receive data
  }
  while(HAL_UART_Receive_IT(&huart3,&USARTLoRa.Rbuffer,1)!=HAL_OK);
}


void IWDGFreezeBySTDBY(void)
{

	 HAL_FLASH_Unlock();

	 __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); //clear FLASH Option validity error flag

	 HAL_FLASH_OB_Unlock();



	 HAL_FLASHEx_OBGetConfig(&pOBInit); //Get Option bit setting


	 //setup IWDG Freeze in standby mode
	 pOBInit.OptionType = OPTIONBYTE_USER;

	 pOBInit.USERType = OB_USER_IWDG_STDBY;

	 pOBInit.USERConfig = OB_IWDG_STDBY_FREEZE;

	 HAL_FLASHEx_OBProgram(&pOBInit);



	 HAL_FLASH_OB_Lock();

	 HAL_FLASH_Lock();
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim: TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim2.Instance)
    {
    	USARTLoRa.CrashTimerCount++;
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
