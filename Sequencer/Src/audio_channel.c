#include "audio_channel.h"

void sequencer_step()
{
  step = (step + 1) % 16;
}

void sequencer_set_pattern(size_t channel, uint16_t pattern)
{
    sequencer[channel].note_on = pattern;
}

void sequencer_set_sample(size_t channel, uint32_t sample_start, uint32_t sample_length)
{
    sequencer[channel].sample_start = (uint32_t)sample_start;
    sequencer[channel].sample_length = (uint32_t)sample_length;
    sequencer[channel].sample_progress = (uint32_t)sample_length;
}


