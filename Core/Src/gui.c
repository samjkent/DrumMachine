#include "gui.h"
#include "spi.h"
#include "ili9341_gfx.h"
#include "ugui.h"
#include "audio_channel.h"

#include <stdio.h>
#include <string.h>

extern int sequencer_channel;

UG_GUI gui;
ILI9341 ili9341;

QueueHandle_t xGUIMsgQueue;
struct GUIMsg
{
    uint8_t id;
    char msg[20];
};

UG_RESULT gui_fill_frame_hw(UG_S16 x1 , UG_S16 y1 , UG_S16 x2 , UG_S16 y2 , UG_COLOR c) {
    ILI9341_Draw_Rectangle(&ili9341, x1, y1, x2+1, y2+1, c);
    return UG_RESULT_OK;
}

UG_RESULT gui_draw_line_hw(UG_S16 x1 , UG_S16 y1 , UG_S16 x2 , UG_S16 y2 , UG_COLOR c) {
    if( y1 == y2 ) {
        ILI9341_Draw_Horizontal_Line(&ili9341, x1, y1, (x2-x1), c);
    } else if( x1 == x2 ) {
        ILI9341_Draw_Vertical_Line(&ili9341, x1, y1, (y2-y1), c);
    } else {
        return UG_RESULT_FAIL;
    }
    return UG_RESULT_OK;
}

void p(UG_S16 x, UG_S16 y, UG_COLOR c) {
      ILI9341_Draw_Pixel(&ili9341, x, y, c);
}

void gui_init() {
  // Init Display Driver
  ILI9341_Struct_Reset(&ili9341);

  ili9341.hspi = &hspi2;

  ili9341.cs_gpio_base = GPIOJ;
  ili9341.cs_gpio_pin = GPIO_PIN_3;
  
  ili9341.dc_gpio_base = GPIOJ;
  ili9341.dc_gpio_pin = GPIO_PIN_4;
  
  ili9341.rst_gpio_base = GPIOH;
  ili9341.rst_gpio_pin = GPIO_PIN_6;

  ili9341.screen_height = 320;
  ili9341.screen_width = 240;
  
  ILI9341_SPI_Init(&ili9341);
  ILI9341_Init(&ili9341);

  ILI9341_Set_Rotation(&ili9341, SCREEN_HORIZONTAL_2);

  // UGUI
  UG_Init(&gui , p, 320, 240 );
  UG_DriverRegister( DRIVER_FILL_FRAME,(void*)gui_fill_frame_hw);
  UG_DriverRegister( DRIVER_DRAW_LINE,(void*)gui_draw_line_hw);

}

void gui_task(void *p) {
  // Set up queue
  xGUIMsgQueue = xQueueCreate( 10, sizeof( struct GUIMsg ) );
  if(xGUIMsgQueue == NULL) {
    println("Failed to allocate xGUIMsgQueue");
  }

  // Set up GUI
  gui_init();
  int duration, start;

  UG_ConsoleSetArea(0,0,320,240);
  UG_FontSelect( &FONT_12X16 ) ;

  UG_ConsoleSetBackcolor( C_BLACK ) ;
  UG_ConsoleSetForecolor( C_WHITE ) ;
  UG_ConsolePutString("Samples\r\n");

  while (1) {
    duration = HAL_GetTick() - start;
    start = HAL_GetTick();

    int nMessages = uxQueueMessagesWaiting(&xGUIMsgQueue);
    while(nMessages > 0) {
        struct GUIMsg pxRxedMessage;
        if( xQueueReceive( xGUIMsgQueue, &( pxRxedMessage ), ( TickType_t ) 10 ) )
        {

            UG_ConsolePutString(pxRxedMessage.msg);
            UG_ConsolePutString("\r\n");

            nMessages--;
        }
    } 

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

/*
void gui_task(void *p) {
  // Init screen
  ILI9341_Struct_Reset(&ili9341);

  ili9341.hspi = &hspi2;

  ili9341.cs_gpio_base = GPIOJ;
  ili9341.cs_gpio_pin = GPIO_PIN_3;
  
  ili9341.dc_gpio_base = GPIOJ;
  ili9341.dc_gpio_pin = GPIO_PIN_4;
  
  ili9341.rst_gpio_base = GPIOH;
  ili9341.rst_gpio_pin = GPIO_PIN_6;

  ili9341.screen_height = 320;
  ili9341.screen_width = 240;
  
  ILI9341_SPI_Init(&ili9341);
  ILI9341_Init(&ili9341);

  ILI9341_Set_Rotation(&ili9341, SCREEN_HORIZONTAL_2);

  int previous_sequencer_channel = -1;

  ILI9341_Fill_Screen(&ili9341, BLACK);

  while (1) {
    // Check if update is required
    if(previous_sequencer_channel != sequencer_channel) {
        // Set previous_sequencer_channel
        previous_sequencer_channel = sequencer_channel;

        // Update sample information
        char channel = sequencer_channel + '0';
        ILI9341_Draw_Text(&ili9341, &channel, 5, 5, GUI_FOREGROUND_COLOUR, 5, GUI_BACKGROUND_COLOUR);
        gui_draw_waveform(sequencer_channel, 0, 80);

        ILI9341_Draw_Text(&ili9341, (char *)"Sample", 5, 140, GUI_FOREGROUND_COLOUR, 2, GUI_BACKGROUND_COLOUR);

        ILI9341_Draw_Horizontal_Line(&ili9341, 80, 140, 230, GUI_FOREGROUND_COLOUR);

    }

    ILI9341_Fill_Screen(&ili9341, BLACK);

    vTaskDelayUntil(40 / portTICK_PERIOD_MS);
  }
}
*/

void gui_draw_ticks(int fs, int div, int size, int windowSize, int x,
                    int yPos) {
  int next = ((x + 1) * windowSize) % (fs / div);
  int curr = ((x)*windowSize) % (fs / div);
  if (next < curr) {
    for (int16_t y = -size; y <= size; y++)
      ILI9341_Draw_Pixel(&ili9341, x, yPos + y, BLUE);
  }
}

void gui_draw_waveform(int track, int channel, int yPos) {

  // Clear waveform if changed
  ILI9341_Draw_Filled_Rectangle_Coord(&ili9341, 0, 40, 320, 130, GUI_BACKGROUND_COLOUR);

  // For x 0->240
  int32_t memory;
  int16_t sample;
  int16_t max, min;
  int16_t prevSample = 0;
  int16_t windowSize = 100;
  uint32_t lastAddress = 0;
  // For x 0->240
  for (int x = 0; x < 300; x++) {
    max = 0;
    min = 0;

    for (int n = 0; n < windowSize; n++) {

      memory =
          *(int32_t *)(sequencer[sequencer_channel].sample_start +
                       (x * 4 * windowSize) +
                       (4 *
                        n)); // + (uint32_t)(x*windowSize) + (uint32_t)(32*n));

      if (channel == 0)
        sample = (int16_t)(memory >> 16);

      if (channel == 1)
        sample = (int16_t)(0xFFFF & memory);

      sample = sample / 640;

      if (sample > max)
        max = sample;
      if (sample < min)
        min = sample;
    }

    // If the window size is large the waveform will not be accurate
    // This provides a better overview of the window
    if (windowSize > 200) {
      for (int16_t y = min; y <= max; y++)
        ILI9341_Draw_Pixel(&ili9341, x + 10, yPos + y, YELLOW);
    } else {

      // Draw wave
      if (prevSample < sample) {
        for (int16_t y = prevSample; y <= sample; y++)
          ILI9341_Draw_Pixel(&ili9341, x + 10, yPos + y, GREEN);
      } else {
        for (int16_t y = prevSample; y >= sample; y--)
          ILI9341_Draw_Pixel(&ili9341, x + 10, yPos + y, GREEN);
      }
    }

    // Draw zero
    ILI9341_Draw_Pixel(&ili9341, x + 10, yPos, BLUE);

    // Draw .1 sec marker
    gui_draw_ticks(44100, 10, 2, windowSize, x, yPos);

    // Draw 1 sec marker
    gui_draw_ticks(44100, 1, 5, windowSize, x, yPos);

    prevSample = sample;
  }
}

void gui_print(char* str) {
    struct GUIMsg msg;
    msg.id = (HAL_GetTick() & 0xFF);
    strcpy(msg.msg, str);
    xQueueSend( xGUIMsgQueue, ( void * ) &msg, ( TickType_t ) 10 );
}

