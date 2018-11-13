#ifndef __AUDIO_CHANNEL_H
#define __AUDIO_CHANNEL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

uint8_t step;

struct audio_channel {
    uint16_t  note_on;
    uint32_t  sample_start;
    uint32_t  sample_length;
    uint32_t  sample_progress;
} audio_channel;

struct audio_channel sequencer[10];

void sequencer_step();
void sequencer_set_pattern(size_t channel, uint16_t pattern);
void sequencer_set_sample(size_t channel, uint32_t sample_start, uint32_t sample_length);
#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHANNEL_H */
