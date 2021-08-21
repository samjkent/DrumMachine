#include "gui.h"
#include "spi.h"
#include "ili9341_gfx.h"
#include "ui.h"
#include "audio_channel.h"

#include "lvgl.h"

#include <stdio.h>
#include <string.h>

extern int sequencer_channel;

QueueHandle_t xGUIMsgQueue;

volatile uint16_t buffer[240][320];
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[240 * 320 / 10];                        /*Declare a buffer for 1/10 screen size*/

static lv_obj_t * meter;

static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_value(meter, indic, v);
}

static lv_disp_drv_t disp_drv;        /*Descriptor of a display driver*/
int y, x;

void gui_start_ili9341() {

  // Init Display Driver
  ili9341.hspi = &hspi2;

  ili9341.cs_gpio_base = GPIOJ;
  ili9341.cs_gpio_pin = GPIO_PIN_3;
  
  ili9341.dc_gpio_base = GPIOJ;
  ili9341.dc_gpio_pin = GPIO_PIN_4;
  
  ili9341.rst_gpio_base = GPIOH;
  ili9341.rst_gpio_pin = GPIO_PIN_6;

  ili9341.screen_height = 240;
  ili9341.screen_width = 320;

  ili9341.buffer = &buffer;
  ili9341.buffer_len = sizeof(buffer);
  
  ILI9341_SPI_Init(&ili9341);
  ILI9341_Init(&ili9341);

  ILI9341_Set_Rotation(&ili9341, SCREEN_HORIZONTAL_2);
    
  ILI9341_StartDMA(&ili9341);

}

static lv_obj_t * list1;

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
    }
}
void lv_example_list_1(void)
{
    /*Create a list*/
    list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, 180, 220);
    lv_obj_center(list1);

    /*Add buttons to the list*/
    lv_obj_t * btn;

    lv_list_add_text(list1, "File");
    btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Connectivity");
    btn = lv_list_add_btn(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Navigation");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Exit");
    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
}

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            ui_draw_pixel(x, y, color_p->full);
            color_p++;
        }
    }

    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void gui_task(void *p) {
  // Set up GUI
  int duration, start;
  int n = 0;  

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, 320 * 240 / 10);  /*Initialize the display buffer.*/

  lv_init();
  lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
  disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;          /*Assign the buffer to the display*/
  disp_drv.hor_res = 320;   /*Set the horizontal resolution of the display*/
  disp_drv.ver_res = 240;   /*Set the verizontal resolution of the display*/
  lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

  lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
  lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
  lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

  lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
  lv_label_set_text(label, "Button");                     /*Set the labels text*/
  lv_obj_center(label);

       meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_indicator_t * indic;

    /*Add a blue arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_var(&a, indic);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_example_list_1();

  while (1) {
    lv_tick_inc(5);
    lv_timer_handler();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

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

void gui_console_reset() {
    struct GUIMsg msg;
    msg.id = (HAL_GetTick() & 0xFF);
    msg.markup = GUI_FLAG_CLEAR;
    xQueueSend( xGUIMsgQueue, ( void * ) &msg, ( TickType_t ) 10 );
}

void gui_reset() {
    struct GUIMsg msg;
    msg.id = (HAL_GetTick() & 0xFF);
    msg.markup = GUI_FLAG_CLEAR;
    xQueueSend( xGUIMsgQueue, ( void * ) &msg, ( TickType_t ) 10 );
}

void gui_print(char* str, uint8_t flags) {
    struct GUIMsg msg;
    msg.id = (HAL_GetTick() & 0xFF);
    msg.markup = flags;
    strcpy(msg.msg, str);
    xQueueSend( xGUIMsgQueue, ( void * ) &msg, ( TickType_t ) 10 );
}

void gui_pot(char* label, uint8_t value, uint8_t max) {
    struct GUIMsg msg;
    msg.id = (HAL_GetTick() & 0xFF);
    msg.markup = GUI_POT;
    strcpy(msg.msg, label);
    msg.values[0] = value;
    msg.values[1] = max;
    xQueueSend( xGUIMsgQueue, ( void * ) &msg, ( TickType_t ) 10 );
}

