/*
 * audio.h
 */ 
#include "audio_channel.h"
#include "cmsis_os.h"
#include "sai.h"
#include "sample_manager.h"
#include "wm8994.h"

#define PLAY_BUFF_SIZE 256
#define NUM_OF_CHANNELS 8

AUDIO_DrvTypeDef *audio_drv;

int16_t SaiBufferSample;

uint32_t playProgress;

TaskHandle_t xAudioBufferManager;

void WM8894_Init();
void audioBufferManager(void *p);

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);

