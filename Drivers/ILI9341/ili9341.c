/* Includes ------------------------------------------------------------------*/
#include "ili9341.h"
#include "spi.h"
#include "gpio.h"

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

uint8_t chunk = 0;
#define DIVIDER  16
#define CHUNK_SIZE ( 153600 / DIVIDER )
void ILI9341_StartDMA(volatile ILI9341* display) {
    if(chunk == DIVIDER) {
        chunk = 0;
    }

    ILI9341_Set_Address(display, 0, chunk * (display->screen_height / DIVIDER), display->screen_width, (chunk + 1) * (display->screen_height / DIVIDER));

    // Data
    HAL_GPIO_WritePin(display->cs_gpio_base, display->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(display->dc_gpio_base, display->dc_gpio_pin, GPIO_PIN_SET);

    // Clean
    SCB_CleanDCache_by_Addr(display->buffer + (chunk*CHUNK_SIZE), CHUNK_SIZE);

    // Send
    HAL_SPI_Transmit_DMA(display->hspi, display->buffer + (chunk*CHUNK_SIZE), CHUNK_SIZE);

    chunk++;
}

