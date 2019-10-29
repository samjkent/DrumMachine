
//    MIT License
//
//    Copyright (c) 2017 Matej Artnak
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
//
//
//-----------------------------------
//    ILI9341 Driver library for STM32
//-----------------------------------
//
//    While there are other libraries for ILI9341 they mostly require either interrupts, DMA or both for fast drawing
//    The intent of this library is to offer a simple yet still reasonably fast alternatives for those that
//    do not wish to use interrupts or DMA in their projects.
//
//    Library is written for STM32 HAL library and supports STM32CUBEMX. To use the library with Cube software
//    you need to tick the box that generates peripheral initialization code in their own respective .c and .h file
//
//
//-----------------------------------
//    Performance
//-----------------------------------
//    Settings:
//    --SPI @ 50MHz
//    --STM32F746ZG Nucleo board
//    --Redraw entire screen
//
//    ++ Theoretical maximum FPS with 50Mhz SPI calculated to be 40.69 FPS
//    ++ 320*240 = 76800 pixels, each pixel contains 16bit colour information (2x8)
//    ++ Theoretical Max FPS: 1/((320*240*16)/50000000)
//
//    With ART Accelerator, instruction prefetch, CPI ICACHE and CPU DCACHE enabled:
//
//    -FPS:             39.62
//    -SPI utilization: 97.37%
//    -MB/Second:       6.09
//
//    With ART Accelerator, instruction prefetch, CPI ICACHE and CPU DCACHE disabled:
//
//    -FPS:             35.45
//    -SPI utilization: 87.12%
//    -MB/Second:       5.44
//
//    ART Accelerator, instruction prefetch, CPI ICACHE and CPU DCACHE settings found in MXCUBE under "System->CORTEX M7 button"
//
//
//
//-----------------------------------
//    How to use this library
//-----------------------------------
//
// @todo UPDATE
//
//-----------------------------------


#ifndef ILI9341_STM32_DRIVER_H
#define ILI9341_STM32_DRIVER_H

#include <stdint.h>

#include "main.h"



#define BURST_MAX_SIZE     500

#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

#define SCREEN_VERTICAL_1   0
#define SCREEN_HORIZONTAL_1 1
#define SCREEN_VERTICAL_2   2
#define SCREEN_HORIZONTAL_2 3



typedef struct
{
    SPI_HandleTypeDef* hspi;

    GPIO_TypeDef* cs_gpio_base;
    uint16_t      cs_gpio_pin;

    GPIO_TypeDef* dc_gpio_base;
    uint16_t      dc_gpio_pin;

    GPIO_TypeDef* rst_gpio_base;
    uint16_t      rst_gpio_pin;

    uint16_t      screen_height;
    uint16_t      screen_width;
} ILI9341;

//! @brief Initializes ILI9341 to zeroized values.
//!
//! @details The intent is to initially set it up this way, then manually populate
//! the necessary values.
void ILI9341_Struct_Reset(volatile ILI9341* display);



//! @brief Sets up GPIO signals for SPI communication.
//!
//! @details Initialize the SPI and GPIO separately beforehand via the generated
//! functions.
void ILI9341_SPI_Init(volatile ILI9341* display);
void ILI9341_SPI_Send(volatile ILI9341* display, unsigned char SPI_Data);
void ILI9341_Write_Command(volatile ILI9341* display, uint8_t Command);
void ILI9341_Write_Data(volatile ILI9341* display, uint8_t Data);
void ILI9341_Set_Address(volatile ILI9341* display, uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void ILI9341_Reset(volatile ILI9341* display);
void ILI9341_Set_Rotation(volatile ILI9341* display, uint8_t Rotation);
void ILI9341_Enable(volatile ILI9341* display);
void ILI9341_Init(volatile ILI9341* display);
void ILI9341_Fill_Screen(volatile ILI9341* display, uint16_t Colour);
void ILI9341_Draw_Colour(volatile ILI9341* display, uint16_t Colour);
void ILI9341_Draw_Pixel(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Colour);
void ILI9341_Draw_Colour_Burst(volatile ILI9341* display, uint16_t Colour, uint32_t Size);


void ILI9341_Draw_Rectangle(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour);
void ILI9341_Draw_Horizontal_Line(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour);
void ILI9341_Draw_Vertical_Line(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour);

#endif
