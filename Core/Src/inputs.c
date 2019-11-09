#include "inputs.h"
#include "cmsis_os.h"
#include "audio_channel.h"

extern int sequencer_channel;

void inputs_setup(void ) {
  mcp23017_init(&hmcp, &hi2c1, MCP23017_ADDRESS_20);

  mcp23017_iocon(&hmcp, MCP23017_PORTA, MCP23017_MIRROR);

  mcp23017_gpinten(&hmcp, MCP23017_PORTA, 0xFF);
  mcp23017_gpinten(&hmcp, MCP23017_PORTB, 0xFF);

  mcp23017_iodir(&hmcp, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(&hmcp, MCP23017_PORTB, MCP23017_IODIR_ALL_INPUT);

  mcp23017_ggpu(&hmcp, MCP23017_PORTA, MCP23017_GPPU_ALL_ENABLED);
  mcp23017_ggpu(&hmcp, MCP23017_PORTB, MCP23017_GPPU_ALL_ENABLED);

  mcp23017_init(&hmcp1, &hi2c1, MCP23017_ADDRESS_21);

  mcp23017_iocon(&hmcp1, MCP23017_PORTA, MCP23017_MIRROR);

  mcp23017_gpinten(&hmcp1, MCP23017_PORTA, 0xFF);
  mcp23017_gpinten(&hmcp1, MCP23017_PORTB, 0xFF);

  mcp23017_iodir(&hmcp1, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(&hmcp1, MCP23017_PORTB, MCP23017_IODIR_ALL_INPUT);

  mcp23017_ggpu(&hmcp1, MCP23017_PORTA, MCP23017_GPPU_ALL_ENABLED);
  mcp23017_ggpu(&hmcp1, MCP23017_PORTB, MCP23017_GPPU_ALL_ENABLED);
}

void inputs_toggle_note(uint8_t n) {
    if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
        set_pixel(n, 0x000000, 0x00FF00);
        sequencer[sequencer_channel].note_on &= ~(1 << n);
    } else {
        set_pixel(n, 0x000F00, 0x00FF00);
        sequencer[sequencer_channel].note_on |= (1 << n);
    }

}

void inputs_check_inputs(void *p) {
  uint8_t lastState[32];
  uint32_t ulNotificationValue;

  xTaskToNotify = xTaskGetCurrentTaskHandle();
        
  // TODO reroute / reorganise to avoid magic numbers / translator
 uint8_t noteKeyTranslate[] = {
                3,7,11,15,
                2,6,10,14,
                1,5,9 ,13,
                0,4,8 ,12
        };

  while (1) {
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, 50 / portTICK_PERIOD_MS);

    // Read rows
    mcp23017_read_intf(&hmcp, MCP23017_PORTA);
    mcp23017_read_intf(&hmcp, MCP23017_PORTB);
    mcp23017_read_intf(&hmcp1, MCP23017_PORTA);
    mcp23017_read_intf(&hmcp1, MCP23017_PORTB);
    // Read rows
    mcp23017_read_intcap(&hmcp, MCP23017_PORTA);
    mcp23017_read_intcap(&hmcp, MCP23017_PORTB);
    mcp23017_read_intcap(&hmcp1, MCP23017_PORTA);
    mcp23017_read_intcap(&hmcp1, MCP23017_PORTB);
    // Read rows
    mcp23017_read_gpio(&hmcp, MCP23017_PORTA);
    mcp23017_read_gpio(&hmcp, MCP23017_PORTB);
    mcp23017_read_gpio(&hmcp1, MCP23017_PORTA);
    mcp23017_read_gpio(&hmcp1, MCP23017_PORTB);

    // INT CAP
    uint32_t currentState = (hmcp.intcap[1] << 24) | (hmcp.intcap[0] << 16) |
                            (hmcp1.intcap[1] << 8) | hmcp1.intcap[0];

    // INTFLAG inverted
    // and then OR'ed with INT CAP
    // Only FLAG = 1 w/ CAP = 0 will remain 0
    uint32_t currentStateIRQ = currentState | ~((hmcp.intf[1] << 24) | (hmcp.intf[0] << 16) |
                            (hmcp1.intf[1] << 8) | hmcp1.intf[0]);

    // Check for channel switch key
    if((currentState >> 20) & 0x01) {
        // Process Note Keys

        for(uint8_t n = 0; n < 16; n++) {
            // Note On / Off 
            if(!((currentStateIRQ >> n) & 0x01)) {
                toggle_key(noteKeyTranslate[n]);
            }
        }
    } else {
        uint8_t curChannel = sequencer_channel;

        // Select new channel
        for(uint8_t n = 0; n < 8; n++) {
            if(!((currentStateIRQ >> n) & 0x01)) {
                sequencer_channel = n;
            }
        }
       
        // Check if changed 
        if(curChannel != sequencer_channel) {
            // Set LEDs
            for(uint8_t n = 0; n < 16; n++) {
                if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
                    set_pixel(n, 0x000F00, 0x00FF00);
                } else {
                    set_pixel(n, 0x000000, 0x00FF00);
                }
            }
        }
    
    }

    // Process Control Keys

    // Process Menu Keys

  }
}
