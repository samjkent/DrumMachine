#include <stdint.h>

// Custom UI Library
#define FONT_WIDTH 8

void ui_draw_string(char* string, int font, int x, int y);
void ui_draw_char(char c, int x, int y);
void ui_draw_pixel(int x, int y, uint16_t color);
void ui_demo();
void ui_draw_box(int x, int y, int x1, int y1, uint16_t color);

