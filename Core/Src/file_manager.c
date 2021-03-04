#include "file_manager.h"
#include "gui.h"
#include "ff_gen_drv.h"
#include "ff.h"
#include "stm32f769i_discovery_qspi.h"
#include "audio_channel.h"
#include "sample_manager.h"

#include <string.h>

extern uint8_t sequencer_channel;

char current_path[30];
uint8_t current_index;
uint8_t max_index;

#define BUFFER_SIZE 512

FRESULT scan_files () {
    FRESULT res;
    DIR dir;
    UINT i = 0;
    static FILINFO fno;

    // Initialise if needed
    if(current_path[0] == '\0') {
        sprintf(&current_path, "0:/");
    }

    res = f_opendir(&dir, current_path);                           /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            memcpy(&directory[i], &fno, sizeof(fno));
            i++;
        }
        f_closedir(&dir);
    }

    // Add end of directory
    max_index = i;
    return res;
}

void attempt_fmount() {
    FRESULT retSD, fr;
    FIL fil;             /* File object */
    UINT br, bw;         /* File read/write count */

    retSD = f_mount(&SDFatFs, SDPath, 1);
    
    // Read Sequencer from FS
    fr = f_open(&fil, "SEQUENCER", FA_READ);
    fr = f_read(&fil, sequencer, sizeof sequencer, &br);
    println("%u", fr);
    fr = f_close(&fil);

}

void file_manager_draw() {
    char title[30];
    sprintf(&title, "File Manager (%u/%u)", current_index, max_index - 1);
    gui_print(title, MARKUP_HEADING);
    for(uint8_t i = current_index; (i < current_index + 15) && (i < (max_index)); i++) {
        if(directory[i].fname == 0) break;
        char fname[256];
        sprintf(&fname, "%s", directory[i].fname);
        if(directory[i].fattrib == AM_DIR) {
            sprintf(&fname, "%s/", fname);
        } else {
            sprintf(&fname, "%s", fname);
        }
        gui_print(fname, (i == current_index ? MARKUP_INVERT : MARKUP_NONE));
    }
}

void file_manager_select() {
    if(directory[current_index].fattrib == AM_DIR) {
        sprintf(&current_path, "%s/%s", current_path, directory[current_index].fname);
        scan_files();
        current_index = 0;
        gui_console_reset();
        file_manager_draw();
    } else {
        gui_print("Loading..", MARKUP_ALERT);
        file_manager_load();
    }
}

void file_manager_index_inc() {
    if(current_index < (max_index - 1)) {
        current_index++;
        gui_console_reset();
        file_manager_draw();
    }
}

void file_manager_index_dec() {
    if(current_index > 0) {
        current_index--;
        gui_console_reset();
        file_manager_draw();
    }
}

void file_manager_directory_up() {
    int length = strlen(current_path);
    for(int n = length; n > 0; n--) {
        if(current_path[n] == '/') {
            current_path[n] = '\0';
            break;
        }
    }
    scan_files();
    current_index = 0;
    gui_console_reset();
    file_manager_draw();
}

void file_manager_load() {
    FIL fil;        /* File object */
    BYTE f_buffer[BUFFER_SIZE];   /* file copy buffer */
    FRESULT fr;     /* FatFs return code */
    UINT br = BUFFER_SIZE;
    int block = 0;
    int ret;

    char path[256];
    sprintf(&path, "%s/%s", current_path, directory[current_index].fname);
    println("%s", path);
    fr = f_open(&fil, path, FA_READ);
    if (fr) {
        gui_print("Error opening file", MARKUP_ALERT);
        println("%u", (int)fr);
        return (int)fr;
    }

    uint8_t mute = 1;
    memcpy(&sequencer[sequencer_channel].mute, &mute, 1);
    memcpy(&sequencer[sequencer_channel].path, &path, 256);
  
    while(br == sizeof f_buffer) { 
        // Read from SD and put into RAM
        f_read(&fil, f_buffer, sizeof f_buffer, &br);

        // Write page
        uint32_t address = (SDRAM_OFFSET * sequencer_channel) + (BUFFER_SIZE * block);

        // Clear page before writing
        if((address % 0x1000) == 0) {
            ret = BSP_QSPI_Erase_Block(address);
            if(ret != QSPI_OK) println("Erase failed: %u Page: %lu", ret, address);
        }

        ret = BSP_QSPI_Write(&f_buffer, address, sizeof f_buffer);
        if(ret != QSPI_OK) println("Write failed: %u Page: %u", ret, address);

        // Block 0 contains WAVE header
        if(!block) {
            memcpy(&sequencer[sequencer_channel].header, &f_buffer, 44);
            char data[4];
            memcpy(&data, sequencer[sequencer_channel].header.SubChunk2ID, 4);
            if(data[0] != 0x64 && data[1] != 0x61 && data[2] != 0x74 && data[3] != 0x61) {
                println("Missing data chunk.");
                // Skip to next
                memcpy(&sequencer[sequencer_channel].header.SubChunk2ID, &f_buffer[44 + sequencer[sequencer_channel].header.Subchunk2Size], 8);
            }
            memcpy(&sequencer[sequencer_channel].sample_progress, &sequencer[sequencer_channel].header.Subchunk2Size, 4);
        }

        block++;
    }
    
    mute = 0;
    memcpy(&sequencer[sequencer_channel].mute, &mute, 1);

    println("Finished reading");
    gui_console_reset();
    file_manager_draw();
    /* Close the file */
    f_close(&fil);

    FIL fil_s;
    // Write sequencer to FS
    fr = f_open(&fil_s, "0:/SEQUENCER", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fil_s, sequencer, sizeof sequencer, &br);
    f_close(&fil_s);

}
