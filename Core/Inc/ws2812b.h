/*
 * WS2182b Driver
 */
#include "main.h"

#define WS2812B_LOW 2
#define WS2812B_HIGH 14

void ws2812b_init();
void ws2812b_set_pixel(uint8_t n, uint32_t grb, uint32_t mask);
