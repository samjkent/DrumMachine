#include <string.h>
#include "ui.h"
#include "ili9341.h"
#include "gui.h"
#include "font.h"

extern volatile uint16_t buffer[240][320];

void ui_draw_string(char* string, int font, int x, int y) {
    for (int i = 0; i < strlen(string); i++) {
        ui_draw_char(string[i], x, y);
        x = x + FONT_WIDTH;
    }
}

void ui_draw_char(char c, int x, int y) {
    for(int _y = 0; _y < 14; _y++) {
        for(int _x = 0; _x < 8; _x++) {
            ui_draw_pixel(x + _x, y + _y, ((font_8x14[c][_y] >> _x) & 0x01) ? 0xFFFF : 0x0000);
        }
    }
}

void ui_fill_screen(uint16_t color) {
    for(int x = 0; x < 320; x++) {
        for(int y = 0; y < 240; y++) {
            ui_draw_pixel(x,y,color);
        }
    }
}

void ui_demo() {
    uint8_t r1, g1, b1, r2, g2, b2, rdiff, gdiff, bdiff;
    r1 = 0x00; g1 = 0x00;  b1 = 0x00;
    r2 = 0xFF; g2 = 0xFF;  b2 = 0xFF;
    rdiff = (r2-r1) / 200;
    gdiff = (g2-g1) / 200;
    bdiff = (b2-b1) / 200;

    for(int x = 0; x < 320; x++) {
        for(int y = 0; y < 240; y++) {
            uint16_t c = (r1 + ((x/10) * rdiff)) << 8 |  
                         (g1 + ((x/10) * gdiff)) << 3 |
                         (b1 + ((x/10) * bdiff)) >> 3;
            ui_draw_pixel(x,y, c);
        }
    }
}

void ui_draw_pixel(int x, int y, uint16_t color) {
    buffer[y][x] = color;
}

void ui_draw_box(int x, int y, int x1, int y1, uint16_t color) {
    for(int _y = y; _y < y1; _y++) {
        for(int _x = x; _x < x1; _x++) {
            ui_draw_pixel(_x, _y, 0xFFFF);
        }
    }
}
