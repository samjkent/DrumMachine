#include "audio_channel.h"

uint8_t sequencer_channel = 0;

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

void sequencer_set_adsr(size_t channel, float a, float d, float s, float r)
{
    sequencer[channel].attack  = a;
    sequencer[channel].decay   = d;
    sequencer[channel].sustain = s;
    sequencer[channel].release = r;
}

void sequencer_calc_adsr(size_t channel)
{
    // Calculate adsr point
    float adsr_point = ((float)sequencer[channel].sample_progress / (float)SAMPLE_RATE);

    // attack
    if(adsr_point <= sequencer[channel].attack){
      // We're on the attack phase
      sequencer[channel].adsr_current = (adsr_point / sequencer[channel].attack);
    }

    // decay
    if(sequencer[channel].attack <= adsr_point && adsr_point <= sequencer[channel].decay)
    {
      // We're in decay
      sequencer[channel].adsr_current = sequencer[channel].sustain + (((adsr_point - sequencer[channel].attack) / sequencer[channel].decay) * (1 - sequencer[channel].sustain));
    }

    sequencer[channel].adsr_current = sequencer[channel].sustain;

}

float sequencer_get_adsr(size_t channel)
{
    return sequencer[channel].adsr_current;
}
