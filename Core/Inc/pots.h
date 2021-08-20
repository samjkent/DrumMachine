/*
 * Potentiometer Handler
 * Uses the devices state to decide how input should be handled
 */
#include "main.h"
#include "cmsis_os.h"

void pots_start_adc();
void pots_update(void *p);
