/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "audio_channel.h"
#include "buttons.h"
#include "cmsis_os.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "ili9341.h"
#include "iwdg.h"
#include "keypad.h"
#include "quadspi.h"
#include "rtc.h"
#include "sample_manager.h"
#include "sai.h"
#include "spdifrx.h"
#include "spi.h"
#include "stm32f7xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "wm8994.h"
#include "ws2812b.h"
#include "wwdg.h"
#include "gui.h"
#include <math.h>

/* Private variables ---------------------------------------------------------*/
#define PLAY_BUFF_SIZE 256

#define ADC_BUFF_SIZE 30 

#define NUM_OF_CHANNELS 8

AUDIO_DrvTypeDef *audio_drv;

enum modes { SEQUENCER, SELECTOR, LIVE };

uint8_t mode = SEQUENCER;

extern TaskHandle_t xTaskToNotify_buttons_read;

extern uint8_t sequencer_channel;

int16_t SaiBuffer[PLAY_BUFF_SIZE];
int16_t SaiBufferSample = 0x0;

uint16_t ADCBuffer_1[ADC_BUFF_SIZE];
uint16_t ADCBuffer_3[ADC_BUFF_SIZE];

volatile int UpdatePointer = -1;
uint32_t playProgress;

int current_step = 0;

volatile uint8_t retVal;

TaskHandle_t xAudioBufferManager;

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_6 || GPIO_Pin == GPIO_PIN_8) {
    if (xTaskToNotify_buttons_read != NULL) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTaskToNotify_buttons_read, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

  } else if (GPIO_Pin == GPIO_PIN_0) {
        // Button on reverse of STM32 DISCO board
  }
}

void WM8894_Init() {
  /* Initialize audio driver */
  if (WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS)) {
    Error_Handler();
  }

  audio_drv = &wm8994_drv;
  audio_drv->Reset(AUDIO_I2C_ADDRESS);
  if (0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 100,
                           AUDIO_FREQUENCY_22K)) {
    Error_Handler();
  }
}

void blinky(void *p) {
  // General task thread
  while (1) {

    char send[80];
    int len = sprintf(&send, "ADC values: %d %d %d %d %d %d\r\n", ADCBuffer_1[0]/41, ADCBuffer_1[1]/41, ADCBuffer_1[2]/41, ADCBuffer_3[0]/41, ADCBuffer_3[1]/41, ADCBuffer_3[2]/41);
    HAL_UART_Transmit(&huart1, &send, len, HAL_MAX_DELAY);

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void semiquaver(void *p) {
  TickType_t xLastWakeTime;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&xLastWakeTime, (114) / portTICK_PERIOD_MS); // 117 = 128 bpm

    for (int i = 0; i < NUM_OF_CHANNELS; i++) {
      if (sequencer[i].note_on & (1 << current_step))
        sequencer[i].sample_progress = (uint32_t)0x00;
    }

    current_step = (current_step + 1) % 16;

    if (mode == SEQUENCER) {
      // Clear blue
      for (int i = 0; i < 16; i++)
        ws2812b_set_pixel(i, 0x000000, 0x0000FF);

      // Set step
      ws2812b_set_pixel(current_step, 0x00000F, 0x0000FF);
    } else if (mode == SELECTOR) {
      // Set LEDs
      for (int i = 0; i < 16; i++) {
        // Clear LEDs
        ws2812b_set_pixel(i, 0x000000, 0xFFFFFF);
      }
      // Set selected sample on
      ws2812b_set_pixel(sequencer_channel, 0x0F0F00, 0xFFFF00);
    }
  }
}

void audioBufferManager(void *p) {
  portTASK_USES_FLOATING_POINT();

  while (1) {
    // Generate samples
    if (UpdatePointer != -1) {
      int pos = UpdatePointer;
      UpdatePointer = -1;

      for (int i = 0; i < PLAY_BUFF_SIZE / 2; i++) {
        SaiBufferSample = 0x00;

        for (int i = 0; i < NUM_OF_CHANNELS; i++) {
          if (sequencer[i].sample_progress < sequencer[i].sample_length) {
            int16_t sample =
                (*(uint16_t *)((uint32_t)sequencer[i].sample_start +
                               (uint32_t)sequencer[i].sample_progress));
            SaiBufferSample += (int16_t)((sample)*sequencer_get_adsr(i));
            sequencer[i].sample_progress = sequencer[i].sample_progress + 4;
          }
        }

        SaiBuffer[pos + i] = SaiBufferSample;
      }

      if (UpdatePointer != -1) {
        // Error_Handler();
      }
    } else {
      // Tasks if we're not updating samples
      for (int j = 0; j < NUM_OF_CHANNELS; j++) {
        sequencer_calc_adsr(j);
      }
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
  UpdatePointer = PLAY_BUFF_SIZE / 2;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) { UpdatePointer = 0; }

void Delay(int counter) {
  while (counter--)
    ;
}


int main(void) {
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI2_Init();

  MX_I2C1_Init();

  ILI9341_Init();

  MX_DMA_Init();
  buttons_init();

  MX_USART1_UART_Init();
  MX_SAI2_Init();
  HAL_SAI_MspInit(&SaiHandle);
  WM8894_Init();

  sample_manager_init();
  
  ws2812b_init();

  MX_ADC1_Init();
  MX_ADC3_Init();
  if(HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(&ADCBuffer_1), ADC_BUFF_SIZE) != HAL_OK) {
          Error_Handler();
  }

  if(HAL_ADC_Start_DMA(&hadc3, (uint32_t *)(&ADCBuffer_3), ADC_BUFF_SIZE) != HAL_OK) {
          Error_Handler();
  }

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  if (0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0)) {
    Error_Handler();
  }

  retVal =
      HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)SaiBuffer, PLAY_BUFF_SIZE);
  if (HAL_OK != retVal)
    Error_Handler();

  xTaskCreate(blinky, (char *)"blinky", 256, NULL, 1, NULL);
  xTaskCreate(semiquaver, (char *)"1/16th Note", 64, NULL, 16, NULL);
  xTaskCreate(audioBufferManager, (char *)"Audio Buffer Manager", 1024, NULL,
              16, NULL);
  xTaskCreate(buttons_read, (char *)"Check Inputs", 256, NULL, 8, NULL);
  xTaskCreate(gui_task, (char *)"GUI Task", 256, NULL, 14, NULL);

  /* Start scheduler */
  osKernelStart();

  /* Infinite loop */
  while (1) {
  }
}

void SystemClock_Config(void) {

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  /**Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_SPDIFRX | RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1 |
      RCC_PERIPHCLK_USART6 | RCC_PERIPHCLK_UART5 | RCC_PERIPHCLK_I2C1 |
      RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_SDMMC2;
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
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure the Systick interrupt time
   */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  /**Configure the Systick
   */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  file: The file name as string.
 * @param  line: The line in file as a number.
 * @retval None
 */
void _Error_Handler(char *file, int line) {
  while (1) {
  }
}

