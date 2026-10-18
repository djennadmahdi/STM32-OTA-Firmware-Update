/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ENABLE_CRC_CHECK          //Activer la vérification CRC
#define ETX_MAGIC_NUMBER            ( 0xFEEDBEEF )//// Nombre magique pour identifier la configuration valide
#define ETX_FWU_CHECK_TIMEOUT_SEC   ( 30 ) //30secs

#define ETX_FWU_FOUND             0
#define ETX_FWU_NOT_FOUND         2

#define ETX_CONFIGURATION_ADDRESS   0x0800C000
#define ETX_CONFIGURATION_SECTOR  FLASH_SECTOR_3

#define ETX_FW_DETAILS_LINK     "https://raw.githubusercontent.com/...../...../...../fw_details.txt"

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
volatile uint16_t time_sec = 0;

/* Slot details */
typedef struct
{
  uint32_t magic_number;                 //Nombre magique pour identifier la configuration valide
  uint32_t slot0_fw_size;                //Taille du firmware/application pour le slot 0
  uint32_t slot0_fw_crc;                 //CRC du firmware/application pour le slot 0
  uint32_t slot1_fw_size;                //Taille du firmware/application pour le slot 1
  uint32_t slot1_fw_crc;                 //CRC du firmware/application pour le slot 1
  uint32_t current_slot;                 //Numéro de slot actif
  uint32_t ota_update_cnt;               //Nombre de mises à jour OTA réussies
}__attribute__((packed)) ETX_FW_INFO_;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
static int check_OTA_GSM( void );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)//est une fonction de rappel (callback) utilisée dans les applications embarquées pour gérer les interruptions générées par un périphérique de timer
{
  time_sec++;
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//SCB->VTOR = 0x08020000;
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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
    {
      printf("Failed to Start the Timer!!! HALT\r\n");
      while(1);
    }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Application 1\r\n...");
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  //printf("Application 2\r\n...");
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
  //printf("Application 3\r\n...");
  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
  while (1)
  {
	  if( time_sec >= ETX_FWU_CHECK_TIMEOUT_SEC )
	      {
	        HAL_TIM_Base_Stop_IT(&htim3);
	        time_sec = 0;

	        if( check_OTA_GSM() == ETX_FWU_FOUND )
	        {
	          printf("OTA Update is available... Rebooting!!!\r\n");
	          NVIC_SystemReset();
	        }

	        	printf("No Update!!!\r\n");
	            HAL_TIM_Base_Start_IT(&htim3);
	        }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1599;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief Print the characters to UART (printf).
  * @retval int
  */
#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the UART2 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

static int check_OTA_GSM( void )
{
  uint8_t   CR                                    = 0x0D;
  int       ret                                   = ETX_FWU_FOUND;
  char      cmd[512];
  char      cmd1[100]                             = {0};
  uint8_t   cnt                                   = 0;
  bool      status                                = false;
  char     *str                                   = NULL;
  uint32_t  fw_size                               = 0;
  uint32_t  fw_crc                                = 0;
  ETX_FW_INFO_ *fw_info                           = (ETX_FW_INFO_*) (ETX_CONFIGURATION_ADDRESS);
  uint32_t  curr_fw_size                          = ( fw_info->current_slot == 0 ) ? fw_info->slot0_fw_size : fw_info->slot1_fw_size;
  uint32_t  curr_fw_crc                           = ( fw_info->current_slot == 0 ) ? fw_info->slot0_fw_crc : fw_info->slot1_fw_crc;

  //Send Carriage Return
  HAL_UART_Transmit(&huart6, (uint8_t *)&CR, 1, HAL_MAX_DELAY);
  HAL_Delay(10);
  printf("Checking OTA update from GSM...\r\n");


  //Turn off the ECHO of GSM
  HAL_UART_Transmit(&huart6, (uint8_t *)"ATE0\r\n", strlen("ATE0\r\n"), 10);
  HAL_Delay(10);

  do
  {
    //Check for GSM Module
    do
    {
      memset(cmd1,0,sizeof(cmd1));
      HAL_UART_Transmit(&huart6, (uint8_t *)"AT\r\n", strlen("AT\r\n"), 1000);
      HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 8, 200 );

      if(strstr((char *)cmd1,"OK"))
      {
        printf("GSM Module Connect...OK\r\n");
        status = true;
        break;
      }
      HAL_Delay(10);
    } while(cnt++ < 5);

    if( !status )
    {
      //No GSM found
      printf("No GSM Module Found\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }

    //Check GSM Network for 20secs
    printf("Scanning Network...\r\n");
    cnt = 0;
    status = false;
    do
    {
      memset(cmd1,0,sizeof(cmd1));
      HAL_UART_Transmit(&huart6, (uint8_t *)"AT+CPSI?\r\n", strlen("AT+CPSI?\r\n"), 1000);
      HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 25, 250 );

      if(!strstr((char *)cmd1,"NO SERVICE"))
      {
        printf("Network Check...OK\r\n");
        status = true;
        break;
      }
      HAL_Delay(1000);
    } while(cnt++ < 10);

    if( !status )
    {
      //No Network found
      printf("No Network Found\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }

    //Turn ON GPRS
    printf("Turning on GPRS...\r\n");
    cnt = 0;
    status = false;
    do
    {
      memset(cmd1,0,sizeof(cmd1));
      HAL_UART_Transmit(&huart6, (uint8_t *)"AT+CGATT=1\r\n", strlen("AT+CGATT=1\r\n"), 1000);
      HAL_Delay(100);
      HAL_UART_Transmit(&huart6, (uint8_t *)"AT+CGATT?\r\n", strlen("AT+CGATT?\r\n"), 1000);
      HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 25, 1000 );

      if(strstr((char *)cmd1,"+CGATT: 1"))
      {
        printf("GPRS On...OK\r\n");
        status = true;
        break;
      }
      printf("GPRS On...Retry %d\r\n", cnt);
      HAL_Delay(1000);
    } while(cnt++ < 5);

    if( !status )
    {
      //No Network found
      printf("GPRS On Failed!!!\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }


    //Start HTTP Service
    cnt = 0;
    status = false;
    do
    {
      memset(cmd1,0,sizeof(cmd1));
      HAL_UART_Transmit(&huart6,(uint8_t *)"AT+HTTPINIT\r\n",strlen("AT+HTTPINIT\r\n"),1000);
      HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 8, 200 );

      if(strstr((char *)cmd1,"OK"))
      {
        //printf("HTTP Init...OK\r\n");
        status = true;
        break;
      }
      else
      {
        HAL_UART_Transmit(&huart6,(uint8_t *)"AT+HTTPTERM\r\n",strlen("AT+HTTPTERM\r\n"),1000);
      }
      HAL_Delay(1000);
    } while(cnt++ < 3);

    if( !status )
    {
      printf("ERR: HTTP Init\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }

    //Read the FW Details
    //Set HTTP address
    memset(cmd1,0,sizeof(cmd1));
    sprintf(cmd,"AT+HTTPPARA=\"URL\",\"%s\"\r\n",ETX_FW_DETAILS_LINK);
    HAL_UART_Transmit(&huart6,(uint8_t *)cmd,strlen(cmd),1000);
    HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 8, 500 );

    if(strstr((char *)cmd1,"OK"))
    {
      //printf("HTTP Server address Set...OK\r\n");
    }
    else
    {
      printf("ERR: HTTP Server address Set\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }
    HAL_Delay(10);

    //HTTP GET
    memset(cmd1,0,sizeof(cmd1));
    HAL_UART_Transmit(&huart6,(uint8_t *)"AT+HTTPACTION=0\r\n",strlen("AT+HTTPACTION=0\r\n"),1000);
    HAL_UART_Receive( &huart6,(uint8_t *)cmd1, 31, 3000 );
    char *ptr = strstr((char *)cmd1,"+HTTPACTION: 0,200,");
    if(!ptr)
    {
      printf("ERR: HTTP Get (FW Details)\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }
    HAL_Delay(100);

    //HTTP Read Firmware Size, CRC
    memset(cmd1,0,sizeof(cmd1));
    HAL_UART_Transmit(&huart6,(uint8_t *)"AT+HTTPREAD=0,30\r\n",strlen("AT+HTTPREAD=0,30\r\n"),1000);
    HAL_UART_Receive( &huart6, (uint8_t *)cmd1, 65, 3000 );

    //Get Size
    str = strstr((char *)cmd1,"SIZ=0x");
    if(str)
    {
      char size[12] = {0};
      strncpy(size, str+4, 10);
      fw_size = (int)strtol(size, NULL, 0);
      printf("New FW Size = %lu bytes\r\n", fw_size);
    }
    else
    {
      printf("No FW Size Found\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }

    //Get CRC
    str = strstr((char *)cmd1,"CRC=0x");
    if(str)
    {
      char crc[12] = {0};
      strncpy(crc, str+4, 10);
      fw_crc = (int)strtoul(crc, NULL, 0);
      printf("FW CRC = 0x%08X\r\n", (unsigned int)fw_crc);
    }
    else
    {
      printf("No FW CRC Found\r\n");
      ret = ETX_FWU_NOT_FOUND;
      break;
    }

    //Compare Fw size with the current one
    if( ( fw_info->magic_number == ETX_MAGIC_NUMBER ) && ( curr_fw_size == fw_size ) )
    {
#ifdef ENABLE_CRC_CHECK
      if( curr_fw_crc == fw_crc )
#endif
      {
        // New version is running already. So no need to update
        printf("Already has latest FW!!!\r\n");
        ret = ETX_FWU_NOT_FOUND;
        break;
      }
    }
    ret = ETX_FWU_FOUND;
  } while( false );

  HAL_UART_Transmit(&huart6,(uint8_t *)"AT+SAPBR=0,1\r\n",strlen("AT+SAPBR=0,1\r\n"),HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart6,(uint8_t *)"AT+HTTPTERM\r\n",strlen("AT+HTTPTERM\r\n"),HAL_MAX_DELAY);

  return ret;
}
//#endif
/* USER CODE END 4 */

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
