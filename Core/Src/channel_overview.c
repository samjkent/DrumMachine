#include "channel_overview.h"
#include "gui.h"
#include "ui.h"
#include "audio_channel.h"
#include "sample_manager.h"
#include <string.h>

extern uint8_t sequencer_channel;

void channel_overview_draw() {
    gui_reset();
    
    // get file name from path
    char filename[32];
    int len = strlen(sequencer[sequencer_channel].path);
    int i = 0;
    for(i = len; i > 0; i--) {
        if(sequencer[sequencer_channel].path[i-1] == '/') break;
    }
    strncpy(&filename, 
            &sequencer[sequencer_channel].path[i],
            len - i);
    
    gui_print(filename, MARKUP_HEADING);
    gui_pot("Vol", 10, 100);
    gui_pot("Start", 10, 100);
    gui_pot("End", 10, 100);

    char info[32];
    snprintf(info, 32, "Sample Rate: %uHz", sequencer[sequencer_channel].header.SampleRate);
    gui_print(info, MARKUP_NONE);
}
