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

void audioBufferManager(void *p) {
  int retVal;
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

  while (1) {
    // Generate samples
    if (UpdatePointer != -1) {
      int pos = UpdatePointer;
      UpdatePointer = -1;

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

      if (UpdatePointer != -1) {
        // Error_Handler();
      }
    } else {
      // Tasks if we're not updating samples
      for (int j = 0; j < NUM_OF_CHANNELS; j++) {
        sequencer_calc_adsr(j);
      }
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
  UpdatePointer = PLAY_BUFF_SIZE / 2;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) { UpdatePointer = 0; }

