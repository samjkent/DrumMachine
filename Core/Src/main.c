/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "quadspi.h"
#include "rtc.h"
#include "sai.h"
#include "spdifrx.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"
#include "wm8994.h"
#include "audio_channel.h"
#include "keypad.h"
#include "ili9341.h"
#include "MCP23017.h"
#include <math.h>
#include "defines.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define PLAY_BUFF_SIZE 256
#define AUDIO_FILE_ADDRESS   0x801002c
// 44 bytes header
#define SINE_ADDRESS   0x802002c

#define ADC_BUFF_SIZE 30

#define NUM_OF_CHANNELS 8
   
AUDIO_DrvTypeDef *audio_drv;

enum modes {
  SEQUENCER,
  SELECTOR,
  LIVE
};


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

// LED mapping required on 0.2 control board
uint8_t ledMapping[25] = { 2,9,16,0, 6,13,20,4, 10,17,1,8, 14,21,5,12, 3,7,10,11,12,13,13,15,2};

uint16_t testData[760] = {
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,

                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2

                        };

TaskHandle_t xAudioBufferManager;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  mode = (mode + 1) % 3;
}

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

void set_pixel(uint8_t n, uint32_t grb, uint32_t mask)
{
    uint8_t m = ledMapping[n];

    // Reverse bit order
    for(int i = 0; i < 24; i++){
        // Check mask for each bit
        if(((mask >> (i)) & 0x01))
            testData[160 + (24 * (m+1)) - (i+1)] = ((grb >> i) & 0x01) ? 6 : 2;
    }
}

void blinky(void *p)
{
        // General task thread
        while(1)
        {
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

void update_step(uint8_t key_pressed)
{ 
   /*
   // Update sequencer
   sequencer_set_pattern(sequencer_channel, (sequencer[sequencer_channel].note_on ^= 1 << key_pressed));

   // Update sequencer LEDs
   if(sequencer[sequencer_channel].note_on & (1 << key_pressed)) {
    set_pixel(key_pressed, 0x000F00, 0x00FF00);
   } else {
    set_pixel(key_pressed, 0x000000, 0x00FF00);
   }
   */
    
   set_pixel(key_pressed, 0x00FF00, 0x00FF00);
}

void matrix(void *p) {
    // Set column high
        // Check each row
            // If high check last state
                // If last state = 0
                    // Set last state = 1
                    // Execute command

    uint8_t last_state[25];

    while(1)
    {
        for(uint8_t col = 0; col <=4; col++) {
            hmcp.gpio[1] = 0x01 << col;
            mcp23017_write_gpio(&hmcp, MCP23017_PORTB);

            vTaskDelay(2 / portTICK_PERIOD_MS);
                    
            // Read rows
            mcp23017_read_gpio(&hmcp, MCP23017_PORTA);
            mcp23017_read_gpio(&hmcp, MCP23017_PORTB);

            // Update last state
            if (hmcp.gpio[1] & (1 << 5)) {
                    last_state[0 + (5 * col)] = (last_state[0 + (5 * col)] << 1) | 0x01;
            } else {
                    last_state[0 + (5 * col)] = (last_state[0 + (5 * col)] << 1);
            }
            if (hmcp.gpio[1] & (1 << 6)) {
                    last_state[1 + (5 * col)] = 0x01;
            } else {
                    last_state[1 + (5 * col)] = 0x00;
            }
            if (hmcp.gpio[1] & (1 << 7)) {
                    last_state[2 + (5 * col)] = 0x01;
            } else {
                    last_state[2 + (5 * col)] = 0x00;
            }
            if (hmcp.gpio[0] & (1 << 0)) {
                    last_state[3 + (5 * col)] = 0x01;
            } else {
                    last_state[3 + (5 * col)] = 0x00;
            }
            if (hmcp.gpio[0] & (1 << 1)) {
                    last_state[4 + (5 * col)] = 0x01;
            } else {
                    last_state[4 + (5 * col)] = 0x00;
            }
        }

        // Use last state to update LEDs
        for(uint8_t n = 0; n < 5; n++) {
            if(last_state[n] == 0x01) {
                    set_pixel(n, 0x000F00, 0x00FF00);
            } else {
                    set_pixel(n, 0x000000, 0x00FF00);
            }
                    
        }
        
        mcp23017_iodir(&hmcp, MCP23017_PORTA, MCP23017_IODIR_ALL_OUTPUT);
        mcp23017_iodir(&hmcp, MCP23017_PORTB, MCP23017_IODIR_ALL_OUTPUT);
  
        mcp23017_iodir(&hmcp, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
        mcp23017_iodir(&hmcp, MCP23017_PORTB, MCP23017_IODIR_ALL_OUTPUT | MCP23017_IODIR_IO5_INPUT | MCP23017_IODIR_IO6_INPUT | MCP23017_IODIR_IO7_INPUT);

        vTaskDelay(2 / portTICK_PERIOD_MS);
    }

}

void check_inputs(void *p)
{
        uint8_t last_pressed = 0xFF;

        uint8_t column = 0;
        // Set column
        hmcp.gpio[1] = 0x01 << 0;
        mcp23017_write_gpio(&hmcp, MCP23017_PORTB);

        while(1)
        {
                // Read rows
                mcp23017_read_gpio(&hmcp, MCP23017_PORTA);
                mcp23017_read_gpio(&hmcp, MCP23017_PORTB);

                // Row 1
                if(hmcp.gpio[1] & (1 << 5) && last_pressed != 0x03) {
                    update_step(column*5 + 0);
                } else if(hmcp.gpio[1] & (1 << 6) && last_pressed != 0x07) {
                    update_step(column*5 + 1);
                } else if(hmcp.gpio[1] & (1 << 7) && last_pressed != 0x0A) {
                    update_step(column*5 + 2);
                } else if(hmcp.gpio[0] & (1 << 0) && last_pressed != 0x0E) {
                    update_step(column*5 + 3);
                } else if(hmcp.gpio[0] & (1 << 1) && last_pressed != 0x02) {
                    update_step(column*5 + 4);
                }

                if(column == 4) {
                    column = 0;
                } else {
                    column++;
                }
                
                // Set column
                hmcp.gpio[1] = 0x01 << column;
                mcp23017_write_gpio(&hmcp, MCP23017_PORTB);
                
            /*    
            uint8_t key_pressed = key_scan();
            char key_uart[10];
            sprintf(key_uart, "Pressed: %d", key_pressed);

            // If a key has been pressed
            switch(mode)
            {
              case SELECTOR:
              {
                  // Select sample
                  if(key_pressed < 8)
                  {
                      //  Choose sample
                      sequencer_channel = key_pressed;
                      
                      break;
                  }

                  // If key is < 8 do function on selected sample
              }
              break;

              case SEQUENCER:
              {
                // Update sequencer
                sequencer_set_pattern(sequencer_channel, (sequencer[sequencer_channel].note_on ^= 1 << key_pressed));
                // Update sequencer LEDs
                for(int i = 0; i < 16; i++){
                    // Clear red note data
                    set_pixel(i, 0x000000, 0x00FF00);

                    // Set if note on
                    if(sequencer[sequencer_channel].note_on & (1 << i))
                        set_pixel(i, 0x000F00, 0x00FF00);
                }
              break;
              }

              case LIVE:
              {
                for(int i = 0; i < 16; i++){
                    // Clear red note data
                    set_pixel(i, 0x000000, 0xFFFFFF);
                }
                sequencer[key_pressed].sample_progress = 0;
                // Set selected note on
                set_pixel(key_pressed, 0x0F0000, 0xFF0000);
              }
            }
            
            */

            vTaskDelay(20 / portTICK_PERIOD_MS);

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
    //HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_13);

    if(mode == SEQUENCER)
    {
      // Clear blue
      for(int i = 0; i < 16; i++)
           set_pixel(i, 0x000000, 0x0000FF);

      // Set step
      set_pixel(current_step, 0x00000F, 0x0000FF);
    } else if(mode == SELECTOR)
    {
      // Set LEDs
      for(int i = 0; i < 16; i++){
        // Clear LEDs
        set_pixel(i, 0x000000, 0xFFFFFF);
      }
      // Set selected sample on
      set_pixel(sequencer_channel, 0x0F0F00, 0xFFFF00);
    }

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
            {
                sequencer_calc_adsr(j);
            }

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

void Delay(int counter)
{
    while(counter--);
}
    
void drawTicks(int fs, int div, int size, int windowSize, int x, int yPos) {
    int next = ((x+1) * windowSize) % (fs / div);
    int curr = ((x) * windowSize) % (fs / div);
    if(next < curr) {
        for(int16_t y = -size; y <= size; y++) 
            ILI9341_DrawPixel(x, yPos + y ,ILI9341_BLUE);
    }
}

void drawChannel(int channel, int yPos) {
  // For x 0->240
  int32_t memory;
  int16_t sample;
  int16_t max, min;
  int16_t prevSample = 0;
  int16_t windowSize = 100;
  uint32_t lastAddress = 0;
  // For x 0->240
  for(int x = 0; x < 300; x++) {
    max = 0; min = 0;

    for(int n = 0; n < windowSize; n++) {

        memory = *(int32_t *) (AUDIO_FILE_ADDRESS + (x * 4 * windowSize) + (4 * n)); // + (uint32_t)(x*windowSize) + (uint32_t)(32*n));
        
        if(channel == 0)
            sample = (int16_t)(memory >> 16);
        
        if(channel == 1)
            sample = (int16_t)(0xFFFF & memory);
    
        sample = sample / 640;
        
        if(sample > max) max = sample;
        if(sample < min) min = sample;
        
    }
    
    // If the window size is large the waveform will not be accurate
    // This provides a better overview of the window 
    if(windowSize > 200) { 
        for(int16_t y = min; y <= max; y++) 
            ILI9341_DrawPixel(x+10, yPos + y ,ILI9341_YELLOW);
    } else {
    
        // Draw wave    
        if(prevSample < sample) {
            for(int16_t y = prevSample; y <= sample; y++) 
                ILI9341_DrawPixel(x+10, yPos + y ,ILI9341_GREEN);
        } else {
            for(int16_t y = prevSample; y >= sample; y--) 
                ILI9341_DrawPixel(x+10, yPos + y ,ILI9341_GREEN);
       }
    }
    
    // Draw zero
    ILI9341_DrawPixel(x+10, yPos ,ILI9341_BLUE);

    // Draw .1 sec marker    
    drawTicks(44100, 10, 2, windowSize, x, yPos);

    // Draw 1 sec marker    
    drawTicks(44100, 1, 5, windowSize, x, yPos);
    
   prevSample = sample;

 }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI2_Init();

  MX_I2C1_Init();

  // MCP Reset
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);

  ILI9341_Init();
  ILI9341_FillScreen(ILI9341_BLACK);
  ILI9341_WriteString(5, 5, (char *)"808_kick.wav", Font_11x18 , ILI9341_WHITE, ILI9341_BLACK);
  drawChannel(0, 80);
 
  ILI9341_WriteString(5, 140, (char *)"Sample", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(5, 150, (char *)"ADSR", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(5, 165, (char *)"Filter", Font_11x18 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(5, 190, (char *)"FX", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(5, 190, (char *)"FX", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  
  ILI9341_DrawHLine(80,140,230, ILI9341_WHITE);

  // Fake filter env
  for(int x = 0; x < 20; x++) {
    ILI9341_DrawPixel(100 + x, 180.0 - (2*x), ILI9341_WHITE);
  }
  for(int x = 0; x < 40; x++) {
    ILI9341_DrawPixel(120 + x, 140, ILI9341_WHITE);
  }
  
  ILI9341_WriteString(170, 140, (char *)"Cut Off: 20 Hz", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(170, 150, (char *)"Resonance: 0%", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(170, 160, (char *)"LFO Amount: 20%", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(170, 170, (char *)"LFO Freq: 1/6", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  ILI9341_WriteString(170, 180, (char *)"Type: High Pass", Font_7x10 , ILI9341_WHITE, ILI9341_BLACK);
  
  // MX_USART1_UART_Init();
  //drawChannel(1, 240);

  
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);

  mcp23017_init(&hmcp, &hi2c1, MCP23017_ADDRESS_20);
  mcp23017_iodir(&hmcp, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(&hmcp, MCP23017_PORTB, MCP23017_IODIR_ALL_OUTPUT | MCP23017_IODIR_IO5_INPUT | MCP23017_IODIR_IO6_INPUT | MCP23017_IODIR_IO7_INPUT);

  MX_DMA_Init();

  // MX_USART1_UART_Init();
  MX_SAI2_Init();
  HAL_SAI_MspInit(&SaiHandle);
  WM8894_Init();

  MX_TIM8_Init();
  HAL_TIM_PWM_Start_DMA (&htim8, TIM_CHANNEL_2, (uint32_t *)(&testData[0]), 760); 
  

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /*
  if(0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0))
  {
      Error_Handler();
  }

  retVal = HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)SaiBuffer, PLAY_BUFF_SIZE);
  if(HAL_OK != retVal)
    Error_Handler();
  */

  sequencer_set_sample(0,  0x2000, 0x10000);
  sequencer_set_adsr(0, 0, 0, .8, 1);
  sequencer[0].note_on = 0xF0;

  sequencer_set_sample(1, 0x16000, 0xF000);
  sequencer_set_adsr(1, 0, 0.2, 0.5, 1);

  sequencer_set_sample(2, 0x2A000, 0xF000);
  sequencer_set_adsr(2, 0, 0.2, 0.5, 1);

  sequencer_set_sample(3, 0x3F000, 0xF000);
  sequencer_set_adsr(3, 0, 0.2, 0.5, 1);

  sequencer_set_sample(4, 0x58000, 0x10000);
  sequencer_set_adsr(4, 0, 0.2, 0.5, 1);

  sequencer_set_sample(5,  0x6B000, 0x2000);
  sequencer_set_adsr(5, 0, 0.2, 0.5, 1);

  sequencer_set_sample(6, 0x81000, 0xF000);
  sequencer_set_adsr(6, 0, 0.2, 0.5, 1);

  sequencer_set_sample(7, 0x28000, 0xF000);
  sequencer_set_adsr(7, 0, 0.2, 0.5, 1);

  // Create two tasks
  /*
  xTaskCreate(blinky, (char*)"blinky", 256, NULL, 1, NULL);
  xTaskCreate(semiquaver, (char*)"1/16th Note", 64, NULL, 16, NULL);
  xTaskCreate(audioBufferManager, (char*)"Audio Buffer Manager", 1024, NULL, 16, NULL);
  */
  xTaskCreate(matrix, (char*)"Check Inputs", 256, NULL, 16, NULL);

  /* Start scheduler */
  osKernelStart();


  /* Infinite loop */
  while (1) { }

}

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

