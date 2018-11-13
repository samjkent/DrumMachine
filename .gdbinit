target remote :4242
load build/FreeRTOS.elf
file build/FreeRTOS.elf
monitor reset
b _Error_Handler
