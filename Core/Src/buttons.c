#include "buttons.h"
#include "audio_channel.h"
#include "i2c.h"
#include "MCP23017.h"
#include "ws2812b.h"
#include "gui.h"
#include "common.h"

#include "file_manager.h"
#include "channel_overview.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

#define CHECK_BUTTON(var,pos) !((var) & (1<<(pos)))  // Active Low

// defines for button states
#define BUTTON_1 12
#define BUTTON_2 8
#define BUTTON_3 4
#define BUTTON_4 0
#define BUTTON_5 13
#define BUTTON_6 9
#define BUTTON_7 5
#define BUTTON_8 1

#define BUTTON_9 14
#define BUTTON_10 10
#define BUTTON_11 6
#define BUTTON_12 2
#define BUTTON_13 15
#define BUTTON_14 11
#define BUTTON_15 7
#define BUTTON_16 3

#define BUTTON_17 19
#define BUTTON_18 18
#define BUTTON_19 17
#define BUTTON_20 16

#define BUTTON_21 23
#define BUTTON_22 22
#define BUTTON_23 21
#define BUTTON_24 20

MCP23017_HandleTypeDef hmcp;
MCP23017_HandleTypeDef hmcp1;

uint8_t enc1;
uint8_t enc1_prev;

static TaskHandle_t xTaskToNotify_buttons_read = NULL;

extern uint8_t sequencer_channel;

extern uint8_t mode;

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
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, 1 / portTICK_PERIOD_MS);

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
    if(!((currentState >> (24)) & 0x01)) {
        file_manager_directory_up();
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
            file_manager_index_inc();
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
            file_manager_index_dec();
            break;
        case 0b1110:
            break;
        case 0b1111:
            break;
    }

    /* Push buttons */
    uint8_t redraw = 0;
    // Note mode
    if(CHECK_BUTTON(currentState, BUTTON_20)) {
        mode = SEQUENCER;
    }

    // Channel select
    if(CHECK_BUTTON(currentStateIRQ, BUTTON_24)) {
        mode = SELECTOR;
        channel_overview_draw();
    }

    // SELECTOR Buttons
    if(mode == SELECTOR) {
        // Select new channel
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_1))
            { sequencer_channel = 0; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_2))
            { sequencer_channel = 1; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_3))
            { sequencer_channel = 2; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_4))
            { sequencer_channel = 3; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_5))
            { sequencer_channel = 4; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_6))
            { sequencer_channel = 5; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_7))
            { sequencer_channel = 6; redraw = 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_8))
            { sequencer_channel = 7; redraw = 1; }

        if(redraw) channel_overview_draw();
    }
    
    // Sequencer Buttons
    if(mode == SEQUENCER) {
        // Select new channel
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_1))
            { sequencer[sequencer_channel].note_on ^= 1 << 0; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_2))
            { sequencer[sequencer_channel].note_on ^= 1 << 1; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_3))
            { sequencer[sequencer_channel].note_on ^= 1 << 2; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_4))
            { sequencer[sequencer_channel].note_on ^= 1 << 3; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_5))
            { sequencer[sequencer_channel].note_on ^= 1 << 4; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_6))
            { sequencer[sequencer_channel].note_on ^= 1 << 5; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_7))
            { sequencer[sequencer_channel].note_on ^= 1 << 6; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_8))
            { sequencer[sequencer_channel].note_on ^= 1 << 7; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_9))
            { sequencer[sequencer_channel].note_on ^= 1 << 8; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_10))
            { sequencer[sequencer_channel].note_on ^= 1 << 9; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_11))
            { sequencer[sequencer_channel].note_on ^= 1 << 10; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_12))
            { sequencer[sequencer_channel].note_on ^= 1 << 11; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_13))
            { sequencer[sequencer_channel].note_on ^= 1 << 12; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_14))
            { sequencer[sequencer_channel].note_on ^= 1 << 13; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_15))
            { sequencer[sequencer_channel].note_on ^= 1 << 14; }
        if(CHECK_BUTTON(currentStateIRQ, BUTTON_16))
            { sequencer[sequencer_channel].note_on ^= 1 << 15; }

        // Set LEDs
        for(uint8_t n = 0; n < 16; n++) {
            if((sequencer[sequencer_channel].note_on >> n) & 0x01) {
                ws2812b_set_pixel(n, 0x000F00, 0x00FF00);
            } else {
                ws2812b_set_pixel(n, 0x000000, 0x00FF00);
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
        println("%i off", n);
    } else {
        ws2812b_set_pixel(n, 0x000F00, 0x00FF00);
        sequencer[sequencer_channel].note_on |= (1 << n);
        println("%i on", n);
    }
}
    
void buttons_notify() {
    if(xTaskToNotify_buttons_read != NULL) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTaskToNotify_buttons_read, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

