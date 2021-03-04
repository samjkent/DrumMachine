#include "main.h"

#define AUDIO_FILE_ADDRESS 0x801002c
#define SAMPLE_REGION_START 0x80000000

struct sample_meta {
  char path[256]; 
  char sample_name[32];
  uint32_t sample_location;
  uint32_t sample_length;
  uint8_t  sample_type;
} sample_meta;

struct sample_meta *sample_manager;

uint32_t number_of_samples;

void sample_manager_init();
void sample_manager_load_table();
