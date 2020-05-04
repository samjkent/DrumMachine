/*
 * Managing button inputs
 */
#include "main.h"
#include "cmsis_os.h"

void buttons_init();
void buttons_read(void *p);
void buttons_notify();
void buttons_toggle(uint8_t n);

