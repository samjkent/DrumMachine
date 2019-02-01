// MCP23017
#include "MCP23017.h"

void MCP23017_init()
{
        // Set up I2C
        GPIO_InitTypeDef GPIO_InitStruct;
        __HAL_RCC_I2C1_CLK_ENABLE();
        /**I2C1 GPIO Configuration
        PB8     ------> I2C1_SCL
        PB9     ------> I2C1_SDA
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        hi2c1.Instance = I2C1;
        hi2c1.Init.ClockSpeed = 100000;
        hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
        hi2c1.Init.OwnAddress1 = 56;
        hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hi2c1.Init.OwnAddress2 = 0;
        hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
        HAL_I2C_Init(&hi2c1);

        // Set up port directions
        uint16_t portDirection = 0xFFFF;
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, IODIRA, I2C_MEMADD_SIZE_8BIT, &portDirection, sizeof(portDirection), HAL_MAX_DELAY );

        // Set up port inversion
        uint16_t portInversion = 0xFFFF;
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, IPOLA, I2C_MEMADD_SIZE_8BIT, &portInversion, sizeof(portInversion), HAL_MAX_DELAY );

        // Set up port interrupt
        uint16_t portInterrupt = 0xFFFF;
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, GPINTENA, I2C_MEMADD_SIZE_8BIT, &portInterrupt, sizeof(portInterrupt), HAL_MAX_DELAY );

        // Set up config
        uint8_t config = 0x4A;
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, IOCONA, I2C_MEMADD_SIZE_8BIT, &config, sizeof(config), HAL_MAX_DELAY );
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, IOCONB, I2C_MEMADD_SIZE_8BIT, &config, sizeof(config), HAL_MAX_DELAY );

        // Set up pull ups
        uint16_t portPU 0xFFFF;
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_DEVICE_ADDRESS, GPPUA, I2C_MEMADD_SIZE_8BIT, &GPPUA, sizeof(GPPUA), HAL_MAX_DELAY );

        // Set up the interrupt GPIO

}

void MCP23017_interrupt()
{
        // If interrupt has triggered something has changed
        // Mem read INTCAPA + B
        // INTERRUPT FLAG REGISTED
        // INTERRUPT CAPTURED REGISTER
        // HAL_I2C_Mem_Read()
}
