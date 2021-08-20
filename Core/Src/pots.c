#include "pots.h"
#include "adc.h"

#define ADC_BUFF_SIZE 30

uint16_t ADCBuffer_1[ADC_BUFF_SIZE];
uint16_t ADCBuffer_3[ADC_BUFF_SIZE];

uint8_t pot_values[6];

void pots_start_adc() {
  // Start the ADCs
  if(HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(&ADCBuffer_1), ADC_BUFF_SIZE) != HAL_OK) {
          Error_Handler();
  }

  if(HAL_ADC_Start_DMA(&hadc3, (uint32_t *)(&ADCBuffer_3), ADC_BUFF_SIZE) != HAL_OK) {
          Error_Handler();
  }
}
  
void pots_update(void *p) {
  while (1) {
    // Check state

    pot_values[0] = ADCBuffer_1[0];
    pot_values[1] = ADCBuffer_1[1];
    pot_values[2] = ADCBuffer_1[2];
   
    pot_values[3] = ADCBuffer_3[0];
    pot_values[4] = ADCBuffer_3[1];
    pot_values[5] = ADCBuffer_3[2];

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
