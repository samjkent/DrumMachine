/*
 * WS2182b Driver
 */
#include "main.h"

#define WS2812B_LOW  5 
#define WS2812B_HIGH 10

void ws2812b_init();
void ws2812b_set_pixel(uint8_t n, uint32_t grb, uint32_t mask);
