#include "audio.h"
#include "audio_channel.h"

extern int sequencer_channel;

void audio_init() {
  portTASK_USES_FLOATING_POINT();
  int update_buffer = 1;

}

void audio_task(void *p) {
  audio_init();

  while (1) {
    // Generate samples
    if (UpdatePointer != -1) {
      int pos = UpdatePointer;
      UpdatePointer = -1;
      fetched_samples = 0;
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

