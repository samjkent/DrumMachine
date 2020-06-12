/*
 * audio.c
 */
#include "audio.h"
extern volatile uint16_t ADCBuffer_1[30];
int8_t qspi_read_buffer[PLAY_BUFF_SIZE];
int16_t SaiBuffer[PLAY_BUFF_SIZE];
int16_t IntermediateSaiBuffer[PLAY_BUFF_SIZE/2];
volatile int UpdatePointer = -1;

void WM8894_Init() {
  /* Initialize audio driver */
  if (WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS)) {
    Error_Handler();
  }

  audio_drv = &wm8994_drv;
  audio_drv->Reset(AUDIO_I2C_ADDRESS);
  if (0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 30,
                           AUDIO_FREQUENCY_44K)) {
    Error_Handler();
  }
}

void audio_task(void *p) {
  int retVal;
  uint32_t ulNotificationValue;
  portTASK_USES_FLOATING_POINT();

  println("Audio Driver");
  if (0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0)) {
    Error_Handler();
  }
  println("SAI init");
  retVal =
      HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)SaiBuffer, PLAY_BUFF_SIZE);
  if (HAL_OK != retVal)
    Error_Handler();
  
  xTaskToNotify_audio_task = xTaskGetCurrentTaskHandle();

  BSP_QSPI_Read(qspi_read_buffer, 0, 44);
  memcpy(&sequencer[0].header, &qspi_read_buffer, 44);
  memcpy(&sequencer[0].sample_progress, &sequencer[0].header.Subchunk2Size, 4);

  while (1) {
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, 2 / portTICK_PERIOD_MS);
    // vTaskDelay(2 / portTICK_PERIOD_MS);

    // Generate samples
    if (UpdatePointer != -1) {
      int pos = UpdatePointer;
      UpdatePointer = -1;
        
      for(int n = 0;  n < (PLAY_BUFF_SIZE / 2); n++) {
            SaiBuffer[pos + n] = 0;
      }
    
      for(int channel = 0; channel < 8; channel++) {

        // Skip mutes
        if(sequencer[channel].mute) continue;

        // Check channel sample rate and bits per sample
        // 44.1 kHz vs 22.05 kHz
        // 8 bps vs 16 vs 24
        // Calculate how much data to read from QSPI to fill PLAY_BUFF_SIZE / 2
        if(sequencer[channel].header.NumChannels == 2) {
            if((sequencer[channel].sample_progress * PLAY_BUFF_SIZE) >= sequencer[channel].header.Subchunk2Size) continue;
            BSP_QSPI_Read(
                    qspi_read_buffer, 
                    44 + (channel * SDRAM_OFFSET) + (4 * ADCBuffer_1[0]) + (sequencer[channel].sample_progress * (2 * (int16_t)(ADCBuffer_1[1] / 16))),
                    PLAY_BUFF_SIZE
            );
            memcpy(&IntermediateSaiBuffer, &qspi_read_buffer, PLAY_BUFF_SIZE );
            for(int i = 0; i < (PLAY_BUFF_SIZE / 2); i++) {
                SaiBuffer[pos + i] += 0.3 * IntermediateSaiBuffer[i];
            }
            sequencer[channel].sample_progress++;
        } else if(sequencer[channel].header.NumChannels == 1) {
            if((sequencer[channel].sample_progress * (PLAY_BUFF_SIZE/2)) >= sequencer[channel].header.Subchunk2Size) continue;
            BSP_QSPI_Read(qspi_read_buffer, 44 + (channel * SDRAM_OFFSET) + (sequencer[channel].sample_progress * ((PLAY_BUFF_SIZE / 2) - 400)), PLAY_BUFF_SIZE / 2);
            memcpy(&IntermediateSaiBuffer, &qspi_read_buffer, PLAY_BUFF_SIZE / 2 );
            for(int i = (PLAY_BUFF_SIZE / 4); i > 0; i--) {
                SaiBuffer[pos + (2 * i)] += 0.3 * IntermediateSaiBuffer[i];
                SaiBuffer[pos + (2 * i) + 1] += 0.3 * IntermediateSaiBuffer[i];
            }
            sequencer[channel].sample_progress++;
        }
      }

      if (UpdatePointer != -1) {
        println("audio task too slow");
        // Error_Handler();
      }

  }
  }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    UpdatePointer = PLAY_BUFF_SIZE / 2;
    if (xTaskToNotify_audio_task != NULL) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTaskToNotify_audio_task, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) { 
    UpdatePointer = 0; 
    if (xTaskToNotify_audio_task != NULL) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTaskToNotify_audio_task, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

