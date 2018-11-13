#include "audio_channel.h"

void sequencer_step()
{
  step = (step + 1) % 16;
}

void sequencer_set_pattern(uint8_t channel, uint16_t pattern)
{
    sequencer[channel].note_on = pattern;
}

void sequencer_set_sample_start(uint8_t channel, uint32_t *sample_start)
{
    sequencer[channel].sample_start = sample_start;
}
