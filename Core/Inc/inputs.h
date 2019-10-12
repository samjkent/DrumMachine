#include "MCP23017.h"

MCP23017_HandleTypeDef hmcp;
MCP23017_HandleTypeDef hmcp1;

// Setup MCP23017 for inputs
void inputs_setup(void);

// Only toggle
void inputs_toggle_key(uint8_t n);

// Task to check inputs from either MCP23017
// Unblocked by an IRQ from MCP23017
void inputs_task(void *p);
