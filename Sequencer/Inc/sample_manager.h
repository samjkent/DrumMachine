#define SAMPLE_REGION_START 0x80000000

struct sample_meta {
  char[32] sample_name;
  uint32_t sample_location;
  uint32_t sample_length;
  uint8_t  sample_type;
} sample_meta;

struct sample_meta *sample_manager;

uint32_t number_of_samples;

void load_sample_table();
