
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "i2c.h"
#include "iwdg.h"
#include "quadspi.h"
#include "rtc.h"
#include "sai.h"
#include "sdmmc.h"
#include "spdifrx.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "wwdg.h"
#include "gpio.h"
#include "fmc.h"
#include "wm8994.h"
#include "audio_channel.h"
#include "keypad.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define PLAY_BUFF_SIZE 256
#define AUDIO_FILE_ADDRESS   0x08010000

#define NUM_OF_CHANNELS 8

AUDIO_DrvTypeDef *audio_drv;

int16_t SaiBuffer[PLAY_BUFF_SIZE];
int16_t SaiBufferSample = 0x0;

volatile int UpdatePointer = -1;
uint32_t playProgress;

int current_step = 0;

volatile uint8_t retVal;

TaskHandle_t xAudioBufferManager;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void WM8894_Init(){
  /* Initialize audio driver */
  if(WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS))
  {
    Error_Handler();
  }

  audio_drv = &wm8994_drv;
  audio_drv->Reset(AUDIO_I2C_ADDRESS);
  if(0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 60, AUDIO_FREQUENCY_22K))
  {
    Error_Handler();
  }
}

void blinky(void *p)
{
        while(1)
        {
            HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_5);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
}

void check_inputs(void *p)
{
        while(1)
        {
            uint8_t key_pressed = key_scan();
            if(key_pressed != 0xFF)
                sequencer_set_pattern(0, (sequencer[0].note_on ^= 1 << key_pressed));

            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
}

void semiquaver(void *p)
{
  TickType_t xLastWakeTime;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while(1)
  {
    vTaskDelayUntil( &xLastWakeTime, (117) / portTICK_PERIOD_MS );

    for(int i = 0; i < NUM_OF_CHANNELS; i++){ 
        if(sequencer[i].note_on & (1 << current_step))
            sequencer[i].sample_progress = (uint32_t)0x00;
    }
    
    current_step = (current_step + 1) % 16;
    HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_13);
  }
}

void audioBufferManager(void *p)
{
        portTASK_USES_FLOATING_POINT();

        while(1){
          // Generate samples
          if(UpdatePointer != -1){
            int pos = UpdatePointer;
            UpdatePointer = -1;
            
                for(int i = 0; i < PLAY_BUFF_SIZE/2; i++)
                {
                    SaiBufferSample = 0x00; 
                    
                    for(int i = 0; i < NUM_OF_CHANNELS; i++)
                    {
                            if(sequencer[i].sample_progress < sequencer[i].sample_length)
                            {
                                int16_t sample   = (*(uint16_t *) (AUDIO_FILE_ADDRESS + (uint32_t)sequencer[i].sample_start + (uint32_t)sequencer[i].sample_progress));
                                SaiBufferSample += (int16_t) ((sample) * sequencer_get_adsr(i));
                                sequencer[i].sample_progress = sequencer[i].sample_progress + 4;
                            }
                    }


                    SaiBuffer[pos + i] = SaiBufferSample;

                }

            if(UpdatePointer != -1)
            {
                Error_Handler();
            }
          } else {
            // Tasks if we're not updating samples
            for(int j = 0; j < NUM_OF_CHANNELS; j++)
                sequencer_calc_adsr(j);
          }


        vTaskDelay(1 / portTICK_PERIOD_MS);

        }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  UpdatePointer = PLAY_BUFF_SIZE/2;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  UpdatePointer = 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  /*
  MX_ADC1_Init();
  MX_ADC3_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_I2C4_Init();
  MX_IWDG_Init();
  MX_QUADSPI_Init();
  MX_RTC_Init();
  MX_SAI2_Init();
  MX_SDMMC2_MMC_Init();
  MX_SPDIFRX_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM12_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_USB_OTG_HS_PCD_Init();
  MX_WWDG_Init();
  */
  /* USER CODE BEGIN 2 */

  MX_SAI2_Init();
  HAL_SAI_MspInit(&SaiHandle);
  WM8894_Init();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  if(0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0))
  {
      Error_Handler();
  }

  retVal = HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)SaiBuffer, PLAY_BUFF_SIZE);
  if(HAL_OK != retVal)
    Error_Handler();

  sequencer_set_sample(0,  0x1500, 0x10000);
  sequencer_set_pattern(0, 0b1000100010001000);
  sequencer_set_adsr(0, 0, 0, .8, 1);

  sequencer_set_sample(1, 0x16000, 0xF000);
  sequencer_set_pattern(1, 0b0000100000001010);
  sequencer_set_adsr(1, 0, 0.2, 0.5, 1);

  sequencer_set_sample(2, 0x26000, 0xF000);
  sequencer_set_pattern(2, 0b0101010101010101);
  sequencer_set_adsr(2, 0, 0.2, 0.5, 1);
  
  sequencer_set_sample(3, 0x36000, 0xF000);
  //sequencer_set_pattern(3, 0b0000001000000010);
  
  sequencer_set_sample(4, 0x50000, 0x1A000);
  //sequencer_set_pattern(4, 0b1010101010101010);
  
  sequencer_set_sample(5,  0x6AD00, 0x2000);
  //sequencer_set_pattern(5, 0xA0A0);

  sequencer_set_sample(6, 0x7F000, 0xF000);
  //sequencer_set_pattern(6, 0x8080);

  sequencer_set_sample(7, 0x26000, 0xF000);
  //sequencer_set_pattern(7, 0x00);
  
  // Create two tasks
  // xTaskCreate(blinky, (char*)"blinky", 64, NULL, 1, NULL);
  xTaskCreate(semiquaver, (char*)"1/16th Note", 64, NULL, 16, NULL);
  xTaskCreate(audioBufferManager, (char*)"Audio Buffer Manager", 1024, NULL, 16, NULL);
  xTaskCreate(check_inputs, (char*)"Check Inputs", 64, NULL, 4, NULL);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART6
                              |RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_SDMMC2;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
