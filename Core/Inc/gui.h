#include "cmsis_os.h"
#include "ili9341.h"

#define GUI_BACKGROUND_COLOUR BLACK
#define GUI_FOREGROUND_COLOUR WHITE

#define MARKUP_NONE    0x0
#define MARKUP_INVERT  0x1
#define MARKUP_HEADING 0x2
#define MARKUP_ALERT 0x3
#define GUI_FLAG_CLEAR 0x4

void gui_init();

void gui_task(void *p);

void gui_draw_ticks(int fs, int div, int size, int windowSize, int x, int yPos);

void gui_draw_waveform(int track, int channel, int yPos);

void gui_console_reset();

void gui_display_thread(void *p);

void gui_print(char* string, uint8_t flags);

void gui_draw_text(char* string, int x, int y, int f, int b);

ILI9341 ili9341;

struct GUIMsg
{
    uint8_t id;
    char markup;
    char msg[256];
};
