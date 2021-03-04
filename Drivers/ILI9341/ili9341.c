
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

/* Includes ------------------------------------------------------------------*/
#include "ili9341.h"
#include "spi.h"
#include "gpio.h"



void ILI9341_Struct_Reset(volatile ILI9341* display)
{
    display->hspi = NULL;

    display->cs_gpio_base = NULL;
    display->cs_gpio_pin  = 0;

    display->dc_gpio_base = NULL;
    display->dc_gpio_pin  = 0;

    display->rst_gpio_base = NULL;
    display->rst_gpio_pin  = 0;

    display->screen_height = 240;
    display->screen_width  = 320;
}



/* Initialize SPI */
void ILI9341_SPI_Init(volatile ILI9341* display)
{
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET); //CS OFF
}

/*Send data (char) to LCD*/
void ILI9341_SPI_Send(volatile ILI9341* display, unsigned char SPI_Data)
{
    HAL_SPI_Transmit(display->hspi, &SPI_Data, 1, 1);
}

/* Send command (char) to LCD */
void ILI9341_Write_Command(volatile ILI9341* display, uint8_t Command)
{
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_RESET);
    ILI9341_SPI_Send(display, Command);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);
}

/* Send Data (char) to LCD */
void ILI9341_Write_Data(volatile ILI9341* display, uint8_t Data)
{
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);
    ILI9341_SPI_Send(display, Data);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);
}

/* Set Address - Location block - to draw into */
void ILI9341_Set_Address(volatile ILI9341* display, uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
    ILI9341_Write_Command(display, 0x2A);
    ILI9341_Write_Data(display, X1>>8);
    ILI9341_Write_Data(display, X1);
    ILI9341_Write_Data(display, X2>>8);
    ILI9341_Write_Data(display, X2);

    ILI9341_Write_Command(display, 0x2B);
    ILI9341_Write_Data(display, Y1>>8);
    ILI9341_Write_Data(display, Y1);
    ILI9341_Write_Data(display, Y2>>8);
    ILI9341_Write_Data(display, Y2);

    ILI9341_Write_Command(display, 0x2C);
}

/*HARDWARE RESET*/
void ILI9341_Reset(volatile ILI9341* display)
{
    HAL_GPIO_WritePin(display->rst_gpio_base, display->rst_gpio_pin, GPIO_PIN_RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(display->rst_gpio_base, display->rst_gpio_pin, GPIO_PIN_SET);
}

/*Ser rotation of the screen - changes x0 and y0*/
void ILI9341_Set_Rotation(volatile ILI9341* display, uint8_t Rotation)
{
    uint8_t screen_rotation = Rotation;

    ILI9341_Write_Command(display, 0x36);
    HAL_Delay(1);

    switch(screen_rotation)
    {
        case SCREEN_VERTICAL_1:
            ILI9341_Write_Data(display, 0x40|0x08);
            display->screen_width = 240;
            display->screen_height = 320;
            break;
        case SCREEN_HORIZONTAL_1:
            ILI9341_Write_Data(display, 0x20|0x08);
            display->screen_width  = 320;
            display->screen_height = 240;
            break;
        case SCREEN_VERTICAL_2:
            ILI9341_Write_Data(display, 0x80|0x08);
            display->screen_width  = 240;
            display->screen_height = 320;
            break;
        case SCREEN_HORIZONTAL_2:
            ILI9341_Write_Data(display, 0x40|0x80|0x20|0x08);
            display->screen_width  = 320;
            display->screen_height = 240;
            break;
        default:
            //EXIT IF SCREEN ROTATION NOT VALID!
            break;
    }
}

/*Enable LCD display*/
void ILI9341_Enable(volatile ILI9341* display)
{
    HAL_GPIO_WritePin(display->rst_gpio_base, display->rst_gpio_pin, GPIO_PIN_SET);
}

/*Initialize LCD display*/
void ILI9341_Init(volatile ILI9341* display)
{
    ILI9341_Enable(display);
    ILI9341_SPI_Init(display);
    ILI9341_Reset(display);

    //SOFTWARE RESET
    ILI9341_Write_Command(display, 0x01);
    HAL_Delay(1000);

    //POWER CONTROL A
    ILI9341_Write_Command(display, 0xCB);
    ILI9341_Write_Data(display, 0x39);
    ILI9341_Write_Data(display, 0x2C);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0x34);
    ILI9341_Write_Data(display, 0x02);

    //POWER CONTROL B
    ILI9341_Write_Command(display, 0xCF);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0xC1);
    ILI9341_Write_Data(display, 0x30);

    //DRIVER TIMING CONTROL A
    ILI9341_Write_Command(display, 0xE8);
    ILI9341_Write_Data(display, 0x85);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0x78);

    //DRIVER TIMING CONTROL B
    ILI9341_Write_Command(display, 0xEA);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0x00);

    //POWER ON SEQUENCE CONTROL
    ILI9341_Write_Command(display, 0xED);
    ILI9341_Write_Data(display, 0x64);
    ILI9341_Write_Data(display, 0x03);
    ILI9341_Write_Data(display, 0x12);
    ILI9341_Write_Data(display, 0x81);

    //PUMP RATIO CONTROL
    ILI9341_Write_Command(display, 0xF7);
    ILI9341_Write_Data(display, 0x20);

    //POWER CONTROL,VRH[5:0]
    ILI9341_Write_Command(display, 0xC0);
    ILI9341_Write_Data(display, 0x23);

    //POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_Write_Command(display, 0xC1);
    ILI9341_Write_Data(display, 0x10);

    //VCM CONTROL
    ILI9341_Write_Command(display, 0xC5);
    ILI9341_Write_Data(display, 0x3E);
    ILI9341_Write_Data(display, 0x28);

    //VCM CONTROL 2
    ILI9341_Write_Command(display, 0xC7);
    ILI9341_Write_Data(display, 0x86);

    //MEMORY ACCESS CONTROL
    ILI9341_Write_Command(display, 0x36);
    ILI9341_Write_Data(display, 0x48);

    //PIXEL FORMAT
    ILI9341_Write_Command(display, 0x3A);
    ILI9341_Write_Data(display, 0x55);

    //FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_Write_Command(display, 0xB1);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0x18);

    //DISPLAY FUNCTION CONTROL
    ILI9341_Write_Command(display, 0xB6);
    ILI9341_Write_Data(display, 0x08);
    ILI9341_Write_Data(display, 0x82);
    ILI9341_Write_Data(display, 0x27);

    //3GAMMA FUNCTION DISABLE
    ILI9341_Write_Command(display, 0xF2);
    ILI9341_Write_Data(display, 0x00);

    //GAMMA CURVE SELECTED
    ILI9341_Write_Command(display, 0x26);
    ILI9341_Write_Data(display, 0x01);

    //POSITIVE GAMMA CORRECTION
    ILI9341_Write_Command(display, 0xE0);
    ILI9341_Write_Data(display, 0x0F);
    ILI9341_Write_Data(display, 0x31);
    ILI9341_Write_Data(display, 0x2B);
    ILI9341_Write_Data(display, 0x0C);
    ILI9341_Write_Data(display, 0x0E);
    ILI9341_Write_Data(display, 0x08);
    ILI9341_Write_Data(display, 0x4E);
    ILI9341_Write_Data(display, 0xF1);
    ILI9341_Write_Data(display, 0x37);
    ILI9341_Write_Data(display, 0x07);
    ILI9341_Write_Data(display, 0x10);
    ILI9341_Write_Data(display, 0x03);
    ILI9341_Write_Data(display, 0x0E);
    ILI9341_Write_Data(display, 0x09);
    ILI9341_Write_Data(display, 0x00);

    //NEGATIVE GAMMA CORRECTION
    ILI9341_Write_Command(display, 0xE1);
    ILI9341_Write_Data(display, 0x00);
    ILI9341_Write_Data(display, 0x0E);
    ILI9341_Write_Data(display, 0x14);
    ILI9341_Write_Data(display, 0x03);
    ILI9341_Write_Data(display, 0x11);
    ILI9341_Write_Data(display, 0x07);
    ILI9341_Write_Data(display, 0x31);
    ILI9341_Write_Data(display, 0xC1);
    ILI9341_Write_Data(display, 0x48);
    ILI9341_Write_Data(display, 0x08);
    ILI9341_Write_Data(display, 0x0F);
    ILI9341_Write_Data(display, 0x0C);
    ILI9341_Write_Data(display, 0x31);
    ILI9341_Write_Data(display, 0x36);
    ILI9341_Write_Data(display, 0x0F);

    //EXIT SLEEP
    ILI9341_Write_Command(display, 0x11);
    HAL_Delay(120);

    //TURN ON DISPLAY
    ILI9341_Write_Command(display, 0x29);

    //STARTING ROTATION
    ILI9341_Set_Rotation(display, SCREEN_VERTICAL_1);
}

//INTERNAL FUNCTION OF LIBRARY, USAGE NOT RECOMMENDED, USE Draw_Pixel INSTEAD
/*Sends single pixel colour information to LCD*/
void ILI9341_Draw_Colour(volatile ILI9341* display, uint16_t Colour)
{
    //SENDS COLOUR
    unsigned char TempBuffer[2] = {Colour>>8, Colour};
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(display->hspi, TempBuffer, 2, 1);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);
}

//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
void ILI9341_Draw_Colour_Burst(volatile ILI9341* display, uint16_t Colour, uint32_t Size)
{
    //SENDS COLOUR
    uint32_t Buffer_Size = 0;
    if((Size*2) < BURST_MAX_SIZE)
    {
        Buffer_Size = Size;
    }
    else
    {
        Buffer_Size = BURST_MAX_SIZE;
    }

    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);

    unsigned char chifted =     Colour>>8;;
    unsigned char burst_buffer[BURST_MAX_SIZE];
    for(uint32_t j = 0; j < Buffer_Size; j+=2)
        {
            burst_buffer[j]   = chifted;
            burst_buffer[j+1] = Colour;
        }

    uint32_t Sending_Size = Size*2;
    uint32_t Sending_in_Block = Sending_Size/Buffer_Size;
    uint32_t Remainder_from_block = Sending_Size%Buffer_Size;

    if(Sending_in_Block != 0)
    {
        for(uint32_t j = 0; j < (Sending_in_Block); j++)
            {
                HAL_SPI_Transmit(display->hspi, (unsigned char *)burst_buffer, Buffer_Size, 10);
            }
    }

    //REMAINDER!
    HAL_SPI_Transmit(display->hspi, (unsigned char *)burst_buffer, Remainder_from_block, 10);

    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);
}

uint8_t chunk = 0;
#define DIVIDER  8
#define CHUNK_SIZE ( 153600 / DIVIDER )
void ILI9341_StartDMA(volatile ILI9341* display, uint8_t* buffer_p) {
    if(chunk == DIVIDER) {
        chunk = 0;
    }

    ILI9341_Set_Address(display, 0, chunk * (display->screen_height / DIVIDER), display->screen_width, (chunk + 1) * (display->screen_height / DIVIDER));

    // Data
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);

    // Clean
    SCB_CleanDCache();
        
    // Send
    HAL_SPI_Transmit_DMA(display->hspi, buffer_p + (chunk*CHUNK_SIZE), CHUNK_SIZE);

    chunk++;
}

//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width amount of colour information to LCD*/
void ILI9341_Fill_Screen(volatile ILI9341* display, uint16_t Colour)
{
    ILI9341_Set_Address(display, 0, 0, display->screen_width, display->screen_height);
    ILI9341_Draw_Colour_Burst(display, Colour, display->screen_width*display->screen_height);
}

//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//
void ILI9341_Draw_Pixel(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Colour)
{
    if((X >=display->screen_width) || (Y >=display->screen_height))
    {
        return;    //OUT OF BOUNDS!
    }

    //ADDRESS
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_RESET);
    ILI9341_SPI_Send(display, 0x2A);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);

    //XDATA
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    unsigned char Temp_Buffer[4] = {X>>8,X, (X+1)>>8, (X+1)};
    HAL_SPI_Transmit(display->hspi, Temp_Buffer, 4, 1);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);

    //ADDRESS
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_RESET);
    ILI9341_SPI_Send(display, 0x2B);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);

    //YDATA
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    unsigned char Temp_Buffer1[4] = {Y>>8,Y, (Y+1)>>8, (Y+1)};
    HAL_SPI_Transmit(display->hspi, Temp_Buffer1, 4, 1);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);

    //ADDRESS
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_RESET);
    ILI9341_SPI_Send(display, 0x2C);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);

    //COLOUR
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    unsigned char Temp_Buffer2[2] = {Colour>>8, Colour};
    HAL_SPI_Transmit(display->hspi, Temp_Buffer2, 2, 1);
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_SET);
}

//DRAW RECTANGLE OF SET SIZE AND HEIGHT AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

void ILI9341_Draw_Rectangle(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
    if((X >=display->screen_width) || (Y >=display->screen_height)) return;
    if((X+Width-1)>=display->screen_width)
    {
        Width=display->screen_width-X;
    }
    if((Y+Height-1)>=display->screen_height)
    {
        Height=display->screen_height-Y;
    }
    ILI9341_Set_Address(display, X, Y, X+Width-1, Y+Height-1);
    ILI9341_Draw_Colour_Burst(display, Colour, Height*Width);
}

void ILI9341_Draw_Filled_Rectangle(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
    if((X >=display->screen_width) || (Y >=display->screen_height)) return;
    if((X+Width-1)>=display->screen_width)
    {
        Width=display->screen_width-X;
    }
    if((Y+Height-1)>=display->screen_height)
    {
        Height=display->screen_height-Y;
    }
    ILI9341_Set_Address(display, X, Y, X+Width-1, Y+Height-1);
    ILI9341_Draw_Colour_Burst(display, Colour, Height*Width);
}

//DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void ILI9341_Draw_Horizontal_Line(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour)
{
    if((X >=display->screen_width) || (Y >=display->screen_height)) return;
    if((X+Width-1)>=display->screen_width)
    {
        Width=display->screen_width-X;
    }
    ILI9341_Set_Address(display, X, Y, X+Width-1, Y);
    ILI9341_Draw_Colour_Burst(display, Colour, Width);
}

//DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void ILI9341_Draw_Vertical_Line(volatile ILI9341* display, uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour)
{
    if((X >=display->screen_width) || (Y >=display->screen_height)) return;
    if((Y+Height-1)>=display->screen_height)
    {
        Height=display->screen_height-Y;
    }
    ILI9341_Set_Address(display, X, Y, X, Y+Height-1);
    ILI9341_Draw_Colour_Burst(display, Colour, Height);
}
