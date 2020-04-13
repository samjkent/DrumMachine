#include "buttons.h"

#include "audio_channel.h"
#include "i2c.h"
#include "MCP23017.h"
#include "ws2812b.h"
#include "file_manager.h"
#include "gui.h"

MCP23017_HandleTypeDef hmcp;
MCP23017_HandleTypeDef hmcp1;

uint8_t enc1;
uint8_t enc1_prev;

extern uint8_t sequencer_channel;

void buttons_init() {
    // Bring up MCP
    // MCP Reset
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_0, GPIO_PIN_RESET);

    // TODO: does this need a delay
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_0, GPIO_PIN_SET);

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

void buttons_read(void *p) {
  uint8_t lastState[32];
  uint32_t ulNotificationValue;

  xTaskToNotify_buttons_read = xTaskGetCurrentTaskHandle();
        
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

    // Encoder 1 pressed
    if(!((currentState >> (25)) & 0x01)) {
        if(!enc1) {
            enc1 = 1;
            file_manager_select();
        }
    } else {
        enc1 = 0;
    }
    // Back button pressed
    if((currentState >> (24)) & 0x01) {

    }
    
    // Check encoder position
    enc1_prev = enc1_prev << 2 | ((currentState >> 27) & 0x01) << 1 | (currentState >> 26) & 0x01;
    switch(enc1_prev & 0xF) {
        case 0b0000:
            break;
        case 0b0001:
            file_manager_index_inc();
            break;
        case 0b0010:
            file_manager_index_dec();
            break;
        case 0b0011:
            break;
        case 0b0100:
            break;
        case 0b0101:
            break;
        case 0b0110:
            break;
        case 0b0111:
            break;
        case 0b1000:
            break;
        case 0b1001:
            break;
        case 0b1010:
            break;
        case 0b1011:
            break;
        case 0b1100:
            break;
        case 0b1101:
            break;
        case 0b1110:
            break;
        case 0b1111:
            break;
    }

    // Check for channel switch key
    if((currentState >> 20) & 0x01) {
        // Process Note Keys

        for(uint8_t n = 0; n < 16; n++) {
            // Note On / Off 
            if(!((currentStateIRQ >> n) & 0x01)) {
                buttons_toggle(noteKeyTranslate[n]);
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
                    ws2812b_set_pixel(n, 0x000F00, 0x00FF00);
                } else {
                    ws2812b_set_pixel(n, 0x000000, 0x00FF00);
                }
            }
        }
    
    }

    // Process Control Keys

    // Process Menu Keys

  }
}

void buttons_toggle(uint8_t n) {
    if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
        ws2812b_set_pixel(n, 0x000000, 0x00FF00);
        sequencer[sequencer_channel].note_on &= ~(1 << n);
    } else {
        ws2812b_set_pixel(n, 0x000F00, 0x00FF00);
        sequencer[sequencer_channel].note_on |= (1 << n);
    }
}
