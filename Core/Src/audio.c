/*
 * audio.c
 */
#include "audio.h"
int16_t SaiBuffer[PLAY_BUFF_SIZE];
volatile int UpdatePointer = -1;

void WM8894_Init() {
  /* Initialize audio driver */
  if (WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS)) {
    Error_Handler();
  }

  audio_drv = &wm8994_drv;
  audio_drv->Reset(AUDIO_I2C_ADDRESS);
  if (0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 50,
                           AUDIO_FREQUENCY_22K)) {
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

  while (1) {
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, 2 / portTICK_PERIOD_MS);

     /*
      for (int i = 0; i < PLAY_BUFF_SIZE / 2; i++) {
        SaiBufferSample = 0x00;

        for (int i = 0; i < NUM_OF_CHANNELS; i++) {
          if (sequencer[i].sample_progress < sequencer[i].sample_length) {
            int16_t sample =
                (*(uint16_t *)((uint32_t)sequencer[i].sample_start +
                               (uint32_t)sequencer[i].sample_progress));
            SaiBufferSample += (int16_t)((sample)*sequencer_get_adsr(i));
            sequencer[i].sample_progress = sequencer[i].sample_progress + 4;
          }
        }

        SaiBuffer[pos + i] = SaiBufferSample;
      }
      */

      if (UpdatePointer != -1) {
        println("audio task too slow");
        // Error_Handler();
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

