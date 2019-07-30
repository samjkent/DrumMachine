// 4x4 Keypad Driver
#include "keypad.h"
#include "usart.h"

GPIO_TypeDef *key_ports[8] = {GPIOJ, GPIOF, GPIOC, GPIOJ,
                              GPIOF, GPIOJ, GPIOC, GPIOC};
uint16_t key_pins[8] = {GPIO_PIN_3, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_0,
                        GPIO_PIN_6, GPIO_PIN_1, GPIO_PIN_6, GPIO_PIN_7};
uint8_t last_pressed = 0xFF;

/* Routine to scan the key pressed */
uint8_t key_scan() {
  uint8_t pressed = 0xff;
  for (uint8_t i = 0; i < 4; i++) {

    keypad_set_row(i);

    for (uint8_t j = 0; j < 4; j++) {
      if (HAL_GPIO_ReadPin(key_ports[4 + j], key_pins[4 + j])) {
        uint8_t ij = ((i * 4) + j);
        pressed = ij; // to send now
      }
    }
  }

  // If nothings been pressed reset
  if (pressed == 0xff) {
    last_pressed = 0xff;
    return 0xff;
  }

  // If nothings changed do nothign but don't reset
  if (last_pressed == pressed) {
    return 0xff; // do nothing
  }

  // If new press return
  last_pressed = pressed;
  return pressed;
}

void keypad_set_row(size_t row) {
  for (uint8_t k = 0; k < 4; k++) {
    if (k == row) {
      HAL_GPIO_WritePin(key_ports[k], key_pins[k], GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(key_ports[k], key_pins[k], GPIO_PIN_RESET);
    }
  }
}

void keypad_clear_last_pressed() {
  //    last_pressed = 0xff;
}
