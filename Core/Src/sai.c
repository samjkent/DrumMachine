/**
  ******************************************************************************
  * File Name          : SAI.c
  * Description        : This file provides code for the configuration
  *                      of the SAI instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "sai.h"

#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
SAI_HandleTypeDef hsai_BlockA2;

SAI_HandleTypeDef SaiHandle;
DMA_HandleTypeDef hSaiDma;

void MX_SAI2_Init(void)
{
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;

  /* Configure PLLSAI prescalers */
  /* PLLSAI_VCO: VCO_429M
     SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
     SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
  RCC_PeriphCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = 429;
  RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = 2;
  RCC_PeriphCLKInitStruct.PLLSAIDivQ = 19;

  if(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initialize SAI */
  __HAL_SAI_RESET_HANDLE_STATE(&SaiHandle);

  SaiHandle.Instance = AUDIO_SAIx;

  __HAL_SAI_DISABLE(&SaiHandle);

  SaiHandle.Init.AudioMode      = SAI_MODEMASTER_TX;
  SaiHandle.Init.Synchro        = SAI_ASYNCHRONOUS;
  SaiHandle.Init.OutputDrive    = SAI_OUTPUTDRIVE_ENABLE;
  SaiHandle.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
  SaiHandle.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
  SaiHandle.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_22K;
  SaiHandle.Init.Protocol       = SAI_FREE_PROTOCOL;
  SaiHandle.Init.DataSize       = SAI_DATASIZE_16;
  SaiHandle.Init.FirstBit       = SAI_FIRSTBIT_MSB;
  SaiHandle.Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;

  SaiHandle.FrameInit.FrameLength       = 32;
  SaiHandle.FrameInit.ActiveFrameLength = 16;
  SaiHandle.FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
  SaiHandle.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
  SaiHandle.FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

  SaiHandle.SlotInit.FirstBitOffset = 0;
  SaiHandle.SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
  SaiHandle.SlotInit.SlotNumber     = 2;
  SaiHandle.SlotInit.SlotActive     = (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1);

  if(HAL_OK != HAL_SAI_Init(&SaiHandle))
  {
    Error_Handler();
  }

  /* Enable SAI to generate clock used by audio driver */
  __HAL_SAI_ENABLE(&SaiHandle);
}

/* SAI1 init function */
// void MX_SAI1_Init(void)
// {
//
//   hsai_BlockA1.Instance = SAI1_Block_A;
//   hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
//   hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
//   hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
//   hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
//   hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
//   hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_192K;
//   hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
//   hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
//   hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
//   hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
//   if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, 2) != HAL_OK)
//   {
//     _Error_Handler(__FILE__, __LINE__);
//   }
//
//   hsai_BlockB1.Instance = SAI1_Block_B;
//   hsai_BlockB1.Init.AudioMode = SAI_MODESLAVE_RX;
//   hsai_BlockB1.Init.Synchro = SAI_SYNCHRONOUS;
//   hsai_BlockB1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
//   hsai_BlockB1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
//   hsai_BlockB1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
//   hsai_BlockB1.Init.MonoStereoMode = SAI_STEREOMODE;
//   hsai_BlockB1.Init.CompandingMode = SAI_NOCOMPANDING;
//   hsai_BlockB1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
//   if (HAL_SAI_InitProtocol(&hsai_BlockB1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, 2) != HAL_OK)
//   {
//     _Error_Handler(__FILE__, __LINE__);
//   }
//
// }
/* SAI2 init function */
/*
void MX_SAI2_Init(void)
{

  hsai_BlockA2.Instance = SAI2_Block_A;
  hsai_BlockA2.Init.Protocol = SAI_SPDIF_PROTOCOL;
  hsai_BlockA2.Init.AudioMode = SAI_MODEMASTER_TX;
  hsai_BlockA2.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
  hsai_BlockA2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
  if (HAL_SAI_Init(&hsai_BlockA2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
*/
static uint32_t SAI1_client =0;
static uint32_t SAI2_client =0;

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  GPIO_Init;

  /* Enable SAI1 clock */
  __HAL_RCC_SAI1_CLK_ENABLE();

  /* Configure GPIOs used for SAI2 */
  AUDIO_SAIx_MCLK_ENABLE();
  AUDIO_SAIx_SCK_ENABLE();
  AUDIO_SAIx_FS_ENABLE();
  AUDIO_SAIx_SD_ENABLE();

  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_Init.Alternate = AUDIO_SAIx_FS_AF;
  GPIO_Init.Pin       = AUDIO_SAIx_FS_PIN;
  HAL_GPIO_Init(AUDIO_SAIx_FS_GPIO_PORT, &GPIO_Init);
  GPIO_Init.Alternate = AUDIO_SAIx_SCK_AF;
  GPIO_Init.Pin       = AUDIO_SAIx_SCK_PIN;
  HAL_GPIO_Init(AUDIO_SAIx_SCK_GPIO_PORT, &GPIO_Init);
  GPIO_Init.Alternate = AUDIO_SAIx_SD_AF;
  GPIO_Init.Pin       = AUDIO_SAIx_SD_PIN;
  HAL_GPIO_Init(AUDIO_SAIx_SD_GPIO_PORT, &GPIO_Init);
  GPIO_Init.Alternate = AUDIO_SAIx_MCLK_AF;
  GPIO_Init.Pin       = AUDIO_SAIx_MCLK_PIN;
  HAL_GPIO_Init(AUDIO_SAIx_MCLK_GPIO_PORT, &GPIO_Init);

  /* Configure DMA used for SAI2 */
  __HAL_RCC_DMA2_CLK_ENABLE();

  if(hsai->Instance == AUDIO_SAIx)
  {
    hSaiDma.Init.Channel             = DMA_CHANNEL_10;
    hSaiDma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hSaiDma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hSaiDma.Init.MemInc              = DMA_MINC_ENABLE;
    hSaiDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hSaiDma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hSaiDma.Init.Mode                = DMA_CIRCULAR;
    hSaiDma.Init.Priority            = DMA_PRIORITY_HIGH;
    hSaiDma.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hSaiDma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hSaiDma.Init.MemBurst            = DMA_MBURST_SINGLE;
    hSaiDma.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    /* Select the DMA instance to be used for the transfer : DMA2_Stream6 */
    hSaiDma.Instance                 = DMA2_Stream6;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmatx, hSaiDma);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hSaiDma);

    /* Configure the DMA Stream */
    if (HAL_OK != HAL_DMA_Init(&hSaiDma))
    {
      Error_Handler();
    }
  }

  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0x01, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}
// void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
// {
//
//   GPIO_InitTypeDef GPIO_InitStruct;
// /* SAI1 */
//     if(hsai->Instance==SAI1_Block_A)
//     {
//     /* SAI1 clock enable */
//     if (SAI1_client == 0)
//     {
//        __HAL_RCC_SAI1_CLK_ENABLE();
//     }
//     SAI1_client ++;
//
//     /**SAI1_A_Block_A GPIO Configuration
//     PE4     ------> SAI1_FS_A
//     PE5     ------> SAI1_SCK_A
//     PE6     ------> SAI1_SD_A
//     PG7     ------> SAI1_MCLK_A
//     */
//     GPIO_InitStruct.Pin = SAI1_FSA_Pin|SAI1_SCKA_Pin|SAI1_SDA_Pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
//     HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//
//     GPIO_InitStruct.Pin = SAI1_MCLKA_Pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
//     HAL_GPIO_Init(SAI1_MCLKA_GPIO_Port, &GPIO_InitStruct);
//
//     }
//     if(hsai->Instance==SAI1_Block_B)
//     {
//       /* SAI1 clock enable */
//       if (SAI1_client == 0)
//       {
//        __HAL_RCC_SAI1_CLK_ENABLE();
//       }
//     SAI1_client ++;
//
//     /**SAI1_B_Block_B GPIO Configuration
//     PE3     ------> SAI1_SD_B
//     */
//     GPIO_InitStruct.Pin = SAI1_SDB_Pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
//     HAL_GPIO_Init(SAI1_SDB_GPIO_Port, &GPIO_InitStruct);
//
//     }
// /* SAI2 */
//     if(hsai->Instance==SAI2_Block_A)
//     {
//     /* SAI2 clock enable */
//     if (SAI2_client == 0)
//     {
//        __HAL_RCC_SAI2_CLK_ENABLE();
//     }
//     SAI2_client ++;
//
//     /**SAI2_A_Block_A GPIO Configuration
//     PD11     ------> SAI2_SD_A
//     */
//     GPIO_InitStruct.Pin = SPDIF_TX_Pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
//     HAL_GPIO_Init(SPDIF_TX_GPIO_Port, &GPIO_InitStruct);
//
//     }
// }

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{

/* SAI1 */
    if(hsai->Instance==SAI1_Block_A)
    {
    SAI1_client --;
    if (SAI1_client == 0)
      {
      /* Peripheral clock disable */
       __HAL_RCC_SAI1_CLK_DISABLE();
      }

    /**SAI1_A_Block_A GPIO Configuration
    PE4     ------> SAI1_FS_A
    PE5     ------> SAI1_SCK_A
    PE6     ------> SAI1_SD_A
    PG7     ------> SAI1_MCLK_A
    */
    HAL_GPIO_DeInit(GPIOE, SAI1_FSA_Pin|SAI1_SCKA_Pin|SAI1_SDA_Pin);

    HAL_GPIO_DeInit(SAI1_MCLKA_GPIO_Port, SAI1_MCLKA_Pin);

    }
    if(hsai->Instance==SAI1_Block_B)
    {
    SAI1_client --;
      if (SAI1_client == 0)
      {
      /* Peripheral clock disable */
      __HAL_RCC_SAI1_CLK_DISABLE();
      }

    /**SAI1_B_Block_B GPIO Configuration
    PE3     ------> SAI1_SD_B
    */
    HAL_GPIO_DeInit(SAI1_SDB_GPIO_Port, SAI1_SDB_Pin);

    }
/* SAI2 */
    if(hsai->Instance==SAI2_Block_A)
    {
    SAI2_client --;
    if (SAI2_client == 0)
      {
      /* Peripheral clock disable */
       __HAL_RCC_SAI2_CLK_DISABLE();
      }

    /**SAI2_A_Block_A GPIO Configuration
    PD11     ------> SAI2_SD_A
    */
    HAL_GPIO_DeInit(SPDIF_TX_GPIO_Port, SPDIF_TX_Pin);

    }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
