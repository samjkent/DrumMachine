#include "sample_manager.h"
#include "audio_channel.h"

void sample_manager_init() {
  sequencer_set_sample(0, AUDIO_FILE_ADDRESS + 0x42, 0x0);
  sequencer_set_adsr(0, 0.2, 1, 1, 2);

  sequencer_set_sample(1, AUDIO_FILE_ADDRESS + 0x15000, 0x8000);
  sequencer_set_adsr(1, 0, 0.2, 0.5, 1);

  sequencer_set_sample(2, AUDIO_FILE_ADDRESS + 0x2A000, 0x4000);
  sequencer_set_adsr(2, 0, 0.2, 0.5, 1);

  sequencer_set_sample(3, AUDIO_FILE_ADDRESS + 0x40000, 0x6000);
  sequencer_set_adsr(3, 0, 0.2, 0.5, 1);

  sequencer_set_sample(4, AUDIO_FILE_ADDRESS + 0x58000, 0x4000);
  sequencer_set_adsr(4, 0, 0.2, 0.5, 1);

  sequencer_set_sample(5, AUDIO_FILE_ADDRESS + 0x80000, 0x8000);
  sequencer_set_adsr(5, 0, 0.2, 0.5, 1);

  sequencer_set_sample(6, AUDIO_FILE_ADDRESS + 0x81000, 0x8000);
  sequencer_set_adsr(6, 0, 0.2, 0.5, 1);

  sequencer_set_sample(7, AUDIO_FILE_ADDRESS + 0x96000, 0x8000);
  sequencer_set_adsr(7, 0, 0.2, 0.5, 1);

}

void sample_manager_load_table(){
  // Load number of samples from 0x80000000
  memcpy(&number_of_samples, SAMPLE_REGION_START, sizeof(uint32_t));

  // Create sample_manager with space for each sample meta
  sample_manager = malloc(number_of_samples * sizeof(sample_meta));

  // Load all samples into sample manager
  memcpy(&sample_manager, SAMPLE_REGION_START + sizeof(uint32_t), number_of_samples * sizeof(sample_meta));

}
