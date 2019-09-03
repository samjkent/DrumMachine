/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "MCP23017.h"
#include "adc.h"
#include "audio_channel.h"
#include "cmsis_os.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "ili9341.h"
#include "iwdg.h"
#include "keypad.h"
#include "quadspi.h"
#include "rtc.h"
#include "sai.h"
#include "spdifrx.h"
#include "spi.h"
#include "stm32f7xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "wm8994.h"
#include "wwdg.h"
#include "gui.h"
#include <math.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define PLAY_BUFF_SIZE 256
#define AUDIO_FILE_ADDRESS 0x801002c
// 44 bytes header
#define SINE_ADDRESS 0x802002c

#define ADC_BUFF_SIZE 30

#define NUM_OF_CHANNELS 8

#define WS2812B_LOW 2
#define WS2812B_HIGH 14

AUDIO_DrvTypeDef *audio_drv;

enum modes { SEQUENCER, SELECTOR, LIVE };

static TaskHandle_t xTaskToNotify = NULL;

uint8_t mode = SEQUENCER;

int16_t SaiBuffer[PLAY_BUFF_SIZE];
int16_t SaiBufferSample = 0x0;

uint32_t ADCBuffer[ADC_BUFF_SIZE];

volatile int UpdatePointer = -1;
uint32_t playProgress;

int current_step = 0;
int sequencer_channel = 0;

volatile uint8_t retVal;

uint32_t globalVolume = 0;

MCP23017_HandleTypeDef hmcp;
MCP23017_HandleTypeDef hmcp1;

// LED mapping required on 0.3 control board
uint8_t ledMapping[25] = {2,  9, 16, 0, 6, 13, 20, 4,  10, 17, 1,  8, 14,
                          21, 5, 12, 7, 3, 22, 18, 23, 19, 15, 11, 24};

uint16_t testData[760] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    // 1
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 2
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 3
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 4
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 5
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 6
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 7
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 8
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 9
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 10
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 11
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 12
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 13
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 14
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 15
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 16
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 17
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 18
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 19
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 20
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 21
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 22
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 23
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 24
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 25
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2

};

TaskHandle_t xAudioBufferManager;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_6 || GPIO_Pin == GPIO_PIN_8) {
    if (xTaskToNotify != NULL) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
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

void set_pixel(uint8_t n, uint32_t grb, uint32_t mask) {
  uint8_t m = ledMapping[n];

  // Reverse bit order
  for (int i = 0; i < 24; i++) {
    // Check mask for each bit
    if (((mask >> (i)) & 0x01))
      testData[160 + (24 * (m + 1)) - (i + 1)] =
          ((grb >> i) & 0x01) ? WS2812B_HIGH : WS2812B_LOW;
  }
}

void blinky(void *p) {
  // General task thread
  while (1) {
    // HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_5);

    // for(int i = 0; i < 3; i++) {
    //     char send = (ADCBuffer[i] >> 24) + 42;
    //     // HAL_UART_Transmit(&huart1, &send, sizeof(send), HAL_MAX_DELAY);
    // }

    // char send = '\n';
    // HAL_UART_Transmit(&huart1, &send, sizeof(send), HAL_MAX_DELAY);

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void update_step(uint8_t key_pressed) {
  /*
  // Update sequencer
  sequencer_set_pattern(sequencer_channel, (sequencer[sequencer_channel].note_on
  ^= 1 << key_pressed));

  // Update sequencer LEDs
  if(sequencer[sequencer_channel].note_on & (1 << key_pressed)) {
   set_pixel(key_pressed, 0x000F00, 0x00FF00);
  } else {
   set_pixel(key_pressed, 0x000000, 0x00FF00);
  }
  */

  set_pixel(key_pressed, 0x00FF00, 0x00FF00);
}

void toggle_key(uint8_t n) {
    if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
        set_pixel(n, 0x000000, 0x00FF00);
        sequencer[sequencer_channel].note_on &= ~(1 << n);
    } else {
        set_pixel(n, 0x000F00, 0x00FF00);
        sequencer[sequencer_channel].note_on |= (1 << n);
    }

}

void check_inputs(void *p) {
  uint8_t lastState[32];
  uint32_t ulNotificationValue;

  xTaskToNotify = xTaskGetCurrentTaskHandle();
        
  // TODO reroute / reorganise to avoid magic numbers / translator
 uint8_t noteKeyTranslate[] = {
                3,7,11,15,
                2,6,10,14,
                1,5,9 ,13,
                0,4,8 ,12
        };

  while (1) {
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, 50 / portTICK_PERIOD_MS);

    // Read rows
    mcp23017_read_intf(&hmcp, MCP23017_PORTA);
    mcp23017_read_intf(&hmcp, MCP23017_PORTB);
    mcp23017_read_intf(&hmcp1, MCP23017_PORTA);
    mcp23017_read_intf(&hmcp1, MCP23017_PORTB);
    // Read rows
    mcp23017_read_intcap(&hmcp, MCP23017_PORTA);
    mcp23017_read_intcap(&hmcp, MCP23017_PORTB);
    mcp23017_read_intcap(&hmcp1, MCP23017_PORTA);
    mcp23017_read_intcap(&hmcp1, MCP23017_PORTB);
    // Read rows
    mcp23017_read_gpio(&hmcp, MCP23017_PORTA);
    mcp23017_read_gpio(&hmcp, MCP23017_PORTB);
    mcp23017_read_gpio(&hmcp1, MCP23017_PORTA);
    mcp23017_read_gpio(&hmcp1, MCP23017_PORTB);

    // INT CAP
    uint32_t currentState = (hmcp.intcap[1] << 24) | (hmcp.intcap[0] << 16) |
                            (hmcp1.intcap[1] << 8) | hmcp1.intcap[0];

    // INTFLAG inverted
    // and then OR'ed with INT CAP
    // Only FLAG = 1 w/ CAP = 0 will remain 0
    uint32_t currentStateIRQ = currentState | ~((hmcp.intf[1] << 24) | (hmcp.intf[0] << 16) |
                            (hmcp1.intf[1] << 8) | hmcp1.intf[0]);

    // Check for channel switch key
    if((currentState >> 20) & 0x01) {
        // Process Note Keys

        for(uint8_t n = 0; n < 16; n++) {
            // Note On / Off 
            if(!((currentStateIRQ >> n) & 0x01)) {
                toggle_key(noteKeyTranslate[n]);
            }
        }
    } else {
        uint8_t curChannel = sequencer_channel;

        // Select new channel
        for(uint8_t n = 0; n < 8; n++) {
            if(!((currentStateIRQ >> n) & 0x01)) {
                sequencer_channel = n;
            }
        }
       
        // Check if changed 
        if(curChannel != sequencer_channel) {
            // Set LEDs
            for(uint8_t n = 0; n < 16; n++) {
                if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
                    set_pixel(n, 0x000F00, 0x00FF00);
                } else {
                    set_pixel(n, 0x000000, 0x00FF00);
                }
            }
        }
    
    }

    // Process Control Keys

    // Process Menu Keys

  }
}

void semiquaver(void *p) {
  TickType_t xLastWakeTime;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&xLastWakeTime, (117) / portTICK_PERIOD_MS); // 117

    for (int i = 0; i < NUM_OF_CHANNELS; i++) {
      if (sequencer[i].note_on & (1 << current_step))
        sequencer[i].sample_progress = (uint32_t)0x00;
    }

    current_step = (current_step + 1) % 16;
    // HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_13);

    if (mode == SEQUENCER) {
      // Clear blue
      for (int i = 0; i < 16; i++)
        set_pixel(i, 0x000000, 0x0000FF);

      // Set step
      set_pixel(current_step, 0x00000F, 0x0000FF);
    } else if (mode == SELECTOR) {
      // Set LEDs
      for (int i = 0; i < 16; i++) {
        // Clear LEDs
        set_pixel(i, 0x000000, 0xFFFFFF);
      }
      // Set selected sample on
      set_pixel(sequencer_channel, 0x0F0F00, 0xFFFF00);
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

  // MCP Reset
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_0, GPIO_PIN_RESET);

  ILI9341_Init();

  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_0, GPIO_PIN_SET);

  mcp23017_init(&hmcp, &hi2c1, MCP23017_ADDRESS_20);

  mcp23017_iocon(&hmcp, MCP23017_PORTA, MCP23017_MIRROR);

  mcp23017_gpinten(&hmcp, MCP23017_PORTA, 0xFF);
  mcp23017_gpinten(&hmcp, MCP23017_PORTB, 0xFF);

  mcp23017_iodir(&hmcp, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(&hmcp, MCP23017_PORTB, MCP23017_IODIR_ALL_INPUT);

  mcp23017_ggpu(&hmcp, MCP23017_PORTA, MCP23017_GPPU_ALL_ENABLED);
  mcp23017_ggpu(&hmcp, MCP23017_PORTB, MCP23017_GPPU_ALL_ENABLED);

  mcp23017_init(&hmcp1, &hi2c1, MCP23017_ADDRESS_21);

  mcp23017_iocon(&hmcp1, MCP23017_PORTA, MCP23017_MIRROR);

  mcp23017_gpinten(&hmcp1, MCP23017_PORTA, 0xFF);
  mcp23017_gpinten(&hmcp1, MCP23017_PORTB, 0xFF);

  mcp23017_iodir(&hmcp1, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(&hmcp1, MCP23017_PORTB, MCP23017_IODIR_ALL_INPUT);

  mcp23017_ggpu(&hmcp1, MCP23017_PORTA, MCP23017_GPPU_ALL_ENABLED);
  mcp23017_ggpu(&hmcp1, MCP23017_PORTB, MCP23017_GPPU_ALL_ENABLED);

  MX_DMA_Init();

  // MX_USART1_UART_Init();
  MX_SAI2_Init();
  HAL_SAI_MspInit(&SaiHandle);
  WM8894_Init();

  MX_TIM8_Init();
  HAL_TIM_PWM_Start_DMA(&htim8, TIM_CHANNEL_2, (uint32_t *)(&testData[0]), 760);

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  if (0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0)) {
    Error_Handler();
  }

  retVal =
      HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)SaiBuffer, PLAY_BUFF_SIZE);
  if (HAL_OK != retVal)
    Error_Handler();

  sequencer_set_sample(0, AUDIO_FILE_ADDRESS + 0x2000, 0x10000);
  sequencer_set_adsr(0, .2, 0, 1, 2);

  sequencer_set_sample(1, AUDIO_FILE_ADDRESS + 0x16000, 0xF000);
  sequencer_set_adsr(1, 0, 0.2, 0.5, 1);

  sequencer_set_sample(2, AUDIO_FILE_ADDRESS + 0x2A000, 0xF000);
  sequencer_set_adsr(2, 0, 0.2, 0.5, 1);

  sequencer_set_sample(3, AUDIO_FILE_ADDRESS + 0x3F000, 0xF000);
  sequencer_set_adsr(3, 0, 0.2, 0.5, 1);

  sequencer_set_sample(4, AUDIO_FILE_ADDRESS + 0x58000, 0x10000);
  sequencer_set_adsr(4, 0, 0.2, 0.5, 1);

  sequencer_set_sample(5, AUDIO_FILE_ADDRESS + 0x6B000, 0x2000);
  sequencer_set_adsr(5, 0, 0.2, 0.5, 1);

  sequencer_set_sample(6, AUDIO_FILE_ADDRESS + 0x81000, 0xF000);
  sequencer_set_adsr(6, 0, 0.2, 0.5, 1);

  sequencer_set_sample(7, AUDIO_FILE_ADDRESS + 0x28000, 0xF000);
  sequencer_set_adsr(7, 0, 0.2, 0.5, 1);

  // Create two tasks
  xTaskCreate(blinky, (char *)"blinky", 256, NULL, 1, NULL);
  xTaskCreate(semiquaver, (char *)"1/16th Note", 64, NULL, 16, NULL);
  xTaskCreate(audioBufferManager, (char *)"Audio Buffer Manager", 1024, NULL,
              16, NULL);
  xTaskCreate(check_inputs, (char *)"Check Inputs", 256, NULL, 16, NULL);
  xTaskCreate(gui_task, (char *)"GUI Task", 256, NULL, 8, NULL);

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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
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
void _Error_Handler(char *file, int line) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
   */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
