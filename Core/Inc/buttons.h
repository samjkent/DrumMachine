/*
 * Managing button inputs
 */
#include "main.h"
#include "cmsis_os.h"

static TaskHandle_t xTaskToNotify_buttons_read = NULL;

void buttons_init();
void buttons_read(void *p);
void buttons_toggle(uint8_t n);

