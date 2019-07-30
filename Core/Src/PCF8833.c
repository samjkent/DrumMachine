#include "PCF8833/h"
#include "gpio.h"
#include "spi.h"

void lcd_init() {
  // Reset
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  HAL_Delay(100);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

  HAL_Delay(100);

  // https://elecfreaks.com/estore/download/EF03044-6100_Display_Driver.pdf
  HAL_SPI_Transmt(&hspi2, 0x11, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x20, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x3A, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x05, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x08, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x25, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x30, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x29, 1, HAL_MAX_DELAY);

  HAL_SPI_Transmt(&hspi2, 0x2A, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x29, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmt(&hspi2, 0x29, 1, HAL_MAX_DELAY);
}
