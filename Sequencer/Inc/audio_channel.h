#ifndef __AUDIO_CHANNEL_H
#define __AUDIO_CHANNEL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

uint8_t step;

struct audio_channel {
    uint16_t  note_on;
    uint32_t *sample_start;
    uint32_t  sample_length;
    uint32_t  sample_progress;
};

struct audio_channel sequencer[8];

void sequencer_step();
void sequencer_set_pattern(uint8_t channel, uint16_t pattern);
void sequencer_set_sample_start(uint8_t channel, uint32_t *sample_start);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHANNEL_H */
