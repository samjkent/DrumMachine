#include "cmsis_os.h"
#include "ili9341.h"

#define GUI_BACKGROUND_COLOUR ILI9341_BLACK
#define GUI_FOREGROUND_COLOUR ILI9341_WHITE

void gui_task(void *p);

void gui_draw_ticks(int fs, int div, int size, int windowSize, int x, int yPos);

void gui_draw_waveform(int track, int channel, int yPos);
