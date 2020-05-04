#include <stdint.h>

// Custom UI Library
#define FONT_WIDTH 8

void ui_draw_string(char* string, int font, int x, int y);
void ui_draw_char(char c, int x, int y);
void ui_draw_pixel(int x, int y, uint16_t color);

