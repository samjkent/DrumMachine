// 4x4 Keypad Driver
#ifndef __keypad_H
#define __keypad_H

#include "main.h"
#include "stm32f7xx_hal.h"

uint8_t key_scan();
void keypad_set_col(size_t col);
void keypad_clear_last_pressed();
#endif
