#define PLAY_BUFF_SIZE 256

TaskHandle_t xAudioBufferManager;

int16_t SaiBuffer[PLAY_BUFF_SIZE];
int16_t SaiBufferSample = 0x0;

void audio_init();
void audio_task(void *p);
