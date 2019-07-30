#include "s1d15g00.h" // LCD-NOKIA6610 Epson:S1D1G00 Controller

void lcd_init() {
  // Reset
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Clock
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Data Out
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Pulse reset
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

  HAL_Delay(200);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  HAL_Delay(200);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

  HAL_Delay(200);

  // Display control
  lcd_write_command(DISCTL);
  lcd_write_data(0x0C); // P1: 0x00 = 2 divisions, switching period=8 (default)
  lcd_write_data(0x20); // P2: 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
  lcd_write_data(0x00); // P3: 0x00 = no inversely highlighted lines

  // COM scan
  lcd_write_command(COMSCN);
  lcd_write_data(1); // P1: 0x01 = Scan 1->80, 160<-81

  // Internal oscilator ON
  lcd_write_command(OSCON);

  // Sleep out
  lcd_write_command(SLPOUT);

  // Power control
  lcd_write_command(PWRCTR);
  lcd_write_data(0x0f); // reference voltage regulator on, circuit voltage
                        // follower on, BOOST ON

  // Inverse display
  lcd_write_command(DISINV);

  // Data control
  lcd_write_command(DATCTL);
  lcd_write_data(0x01); // P1: 0x01 = page address inverted, column address
                        // normal, address scan in column direction
  lcd_write_data(0x00); // P2: 0x00 = RGB sequence (default value)
  lcd_write_data(
      0x02); // P3: 0x02 = Grayscale -> 16 (selects 12-bit color, type A)

  // Voltage control (contrast setting)
  lcd_write_command(VOLCTR);
  lcd_write_data(32); // P1 = 32 volume value (experiment with this value to get
                      // the best contrast)
  lcd_write_data(3);  // P2 = 3 resistance ratio (only value that works)

  // allow power supply to stabilize
  HAL_Delay(1000);

  // turn on the display
  lcd_write_command(DISON);
}

void lcd_write(uint8_t notC_D, uint8_t data) {
  uint8_t command_byte = 0x00;

  for (int bit = 0; bit < 9; bit++) {
    // Clear clock
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

    // Half Period
    HAL_Delay(1);

    // Command or Data
    if (bit == 0) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, notC_D);
    } else {
      uint8_t dataOut = (data << (bit - 1)) & 0x08;
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, dataOut);
    }

    // Set clock
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);

    // Half Period
    HAL_Delay(1);
  }

  /*
  HAL_SPI_Transmit(&hspi2, &command_byte, 1, HAL_MAX_DELAY);
  while(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);

  HAL_SPI_Transmit(&hspi2, &command, 1, HAL_MAX_DELAY);
  while(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
  */
}

void lcd_write_command(uint8_t data) { lcd_write(0x00, data); }

void lcd_write_data(uint8_t data) { lcd_write(0x01, data); }

void lcd_set_pixel(int x, int y, int color) {
  // Row address set (command 0x2B)
  lcd_write_command(PASET);
  lcd_write_data(x);
  lcd_write_data(x);

  // Column address set (command 0x2A)
  lcd_write_command(CASET);
  lcd_write_data(y);
  lcd_write_data(y);

  // Now illuminate the pixel (2nd pixel will be ignored)
  lcd_write_command(RAMWR);
  lcd_write_data((color >> 4) & 0xFF);
  lcd_write_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
  lcd_write_data(color & 0xFF);
}
