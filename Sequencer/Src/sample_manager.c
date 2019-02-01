#include "sample_manager.h"

void load_sample_table(){
  // Load number of samples from 0x80000000
  memcpy(&number_of_samples, SAMPLE_REGION_START, sizeof(uint32_t));

  // Create sample_manager with space for each sample meta
  sample_manager = malloc(number_of_samples * sizeof(sample_meta));

  // Load all samples into sample manager
  memcpy(&sample_manager, SAMPLE_REGION_START + sizeof(uint32_t), number_of_samples * sizeof(sample_meta));

}
