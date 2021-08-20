/* Includes ------------------------------------------------------------------*/
#include "spi.h"

#include "gpio.h"
#include "gui.h"
#include "ili9341.h"

/* USER CODE BEGIN 0 */
extern uint16_t buffer;
/* USER CODE END 0 */

SPI_HandleTypeDef hspi2;

DMA_HandleTypeDef hdma_spi2;

/* SPI2 init function */
void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    /**SPI2 GPIO Configuration
    PA12     ------> SPI2_SCK
    PA11     ------> SPI2_NSS
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */

    // SCK
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // MOSI
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
    // CS 
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // digital Output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
  
    // Reset
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // digital Output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    // DC
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // digital Output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
  

  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_spi2.Instance = DMA1_Stream4;
  hdma_spi2.State = HAL_DMA_STATE_READY;
  hdma_spi2.Init.Channel = DMA_CHANNEL_0;
  hdma_spi2.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_spi2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_spi2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_spi2.Init.Priority = DMA_PRIORITY_LOW;
  hdma_spi2.Init.Mode = DMA_NORMAL;
  hdma_spi2.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_spi2.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  if (HAL_DMA_Init(&hdma_spi2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  __HAL_LINKDMA(&hspi2,hdmatx,hdma_spi2);
  
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PA12     ------> SPI2_SCK
    PA11     ------> SPI2_NSS
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, ARD_D13_SCK_Pin|SPI2_NSS_Pin);

    HAL_GPIO_DeInit(GPIOB, ARDUINO_MISO_D12_Pin|ARDUINO_MOSI_PWM_D11_Pin);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}


/* USER CODE BEGIN 1 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
    if(hspi->Instance == SPI2) {
        ILI9341_StartDMA(&ili9341);
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi) {
    if(hspi->Instance == SPI2) {
        println("SPI2 Error: 0x%x, DMA: 0x%x", hspi2.ErrorCode, hdma_spi2.ErrorCode);
    }
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
