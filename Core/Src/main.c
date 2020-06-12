/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "audio.h"
#include "buttons.h"
#include "cmsis_os.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "ili9341.h"
#include "iwdg.h"
#include "keypad.h"
#include "file_manager.h"
#include "quadspi.h"
#include "rtc.h"
#include "sdmmc.h"
#include "ff_gen_drv.h"
#include "ff.h"
#include "spdifrx.h"
#include "spi.h"
#include "stm32f7xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "ws2812b.h"
#include "wwdg.h"
#include "gui.h"
#include <math.h>

#include "stm32f769i_discovery_sd.h"
#include "stm32f769i_discovery_qspi.h"

/* Private variables ---------------------------------------------------------*/
#define ADC_BUFF_SIZE 30 
#define NUM_OF_CHANNELS 8

FATFS SDFatFs;
FIL file;
extern Diskio_drvTypeDef SD_Driver;
char SDPath[4];

enum modes { SEQUENCER, SELECTOR, LIVE };

uint8_t mode = SEQUENCER;

extern uint8_t sequencer_channel;

volatile uint16_t ADCBuffer_1[ADC_BUFF_SIZE];
volatile uint16_t ADCBuffer_3[ADC_BUFF_SIZE];

int current_step = 0;

volatile uint8_t retVal;

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_6 || GPIO_Pin == GPIO_PIN_8) {
        buttons_notify();
  } else if (GPIO_Pin == GPIO_PIN_0) {
        // Button on reverse of STM32 DISCO board
  }
}


void blinky(void *p) {
  // General task thread
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  attempt_fmount();
  scan_files();
  file_manager_draw();
  while (1) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    println("%u %u %u %u %u %u", ADCBuffer_1[0], ADCBuffer_1[1], ADCBuffer_1[2], ADCBuffer_3[0], ADCBuffer_3[1], ADCBuffer_3[2]);
  }
}

#define portTICK_PERIOD_US			( ( TickType_t ) 1000000 / configTICK_RATE_HZ )
void semiquaver(void *p) {
  TickType_t xLastWakeTime;
  portTASK_USES_FLOATING_POINT();
  
  int bpm = ((60.0/87.0)/4.0)*1000000;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&xLastWakeTime,  bpm / portTICK_PERIOD_US); // 117 = 128 bpm

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

void Delay(int counter) {
  while (counter--)
    ;
}

void format_disk() {
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw;            /* Bytes written */
    BYTE work[_MAX_SS]; /* Work area (larger is better for processing time) */

    printf("start format \r\n");
    /* Format the default drive with default parameters */
    res = f_mkfs("", FM_ANY, 0, work, sizeof work);
    if (res) {
        printf("work %d mkfs failed %d \r\n", _MAX_SS, res);
    }
    printf("format successful \r\n");

    /* Gives a work area to the default drive */
    res = f_mount(&SDFatFs, "0:/", 1);
    printf("fmount res %d \r\n", res);

    /* Create a file as new */
    res = f_open(&fil, "0:/hello.txt", FA_OPEN_ALWAYS
                    | FA_WRITE);
    if (res) {
        printf("open error\r\n");
    }

    /* Close the file */
    f_close(&fil);

    /* Unregister work area */
    f_mount(0, "", 0);
}

int main(void) {
  HAL_Init();
  SystemClock_Config();

  SCB_EnableICache();
  SCB_EnableDCache();

  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();

  MX_I2C1_Init();

  MX_DMA_Init();
  buttons_init();

  MX_SAI2_Init();
  HAL_SAI_MspInit(&SaiHandle);
  WM8894_Init();

  sample_manager_init();
  
  ws2812b_init();

  BSP_QSPI_Init();

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

  println("xTaskCreate");
  xTaskCreate(gui_task, (char *)"GUI Task", 256, NULL, 8, NULL);
  xTaskCreate(semiquaver, (char *)"1/16th Note", 256, NULL, 8, NULL);
  xTaskCreate(audio_task, (char *)"Audio Buffer Manager", 256, NULL, 15, NULL);
  xTaskCreate(buttons_read, (char *)"Check Inputs", 1024, NULL, 8, NULL);
  xTaskCreate(blinky, (char *)"blinky", 1024, NULL, 8, NULL);
 
  uint8_t ret;
  sprintf(SDPath, "0:");
  ret = FATFS_LinkDriver(&SD_Driver, SDPath);
  printf("FATFS_LinkDriver() returns %d \r\n", ret);

  /* Start scheduler */
  println("osKernelStart()");
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

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  if(HAL_PWREx_EnableOverDrive() != HAL_OK) {
          Error_Handler();
  }

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_SPDIFRX | RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1 |
      RCC_PERIPHCLK_USART6 | RCC_PERIPHCLK_UART5 | RCC_PERIPHCLK_I2C1 |
      RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_SDMMC2 | RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV4;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
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
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
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

/**
 * Redirect printf to UART1
 */
int _write(int file, char* data, int len)
{
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, data, len, 0xFFFF);
    return (status == HAL_OK ? len : 0);
}

