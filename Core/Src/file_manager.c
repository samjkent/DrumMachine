#include "file_manager.h"
#include "gui.h"
#include "ff_gen_drv.h"
#include "ff.h"
#include "stm32f769i_discovery_qspi.h"
#include "audio_channel.h"

extern uint8_t sequencer_channel;

char current_path[30];
uint8_t current_index;
uint8_t max_index;

void attempt_fmount();
FRESULT scan_files();

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
    FRESULT retSD;
    FIL fil;             /* File object */
    UINT br, bw;         /* File read/write count */

    retSD = f_mount(&SDFatFs, SDPath, 1);

}

void file_manager_draw() {
    // gui_console_reset();
    char title[30];
    sprintf(&title, "File Manager (%u/%u)", current_index, max_index - 1);
    gui_print(title, MARKUP_HEADING);
    for(uint8_t i = current_index; (i < current_index + 15) && (i < (max_index)); i++) {
        if(directory[i].fname == 0) break;
        char fname[30];
        sprintf(&fname, "%s", directory[i].fname);
        if(directory[i].fattrib == AM_DIR) {
            sprintf(&fname, "%s/", fname);
        } else {
            sprintf(&fname, "%s   %ukB", fname, (int)(directory[i].fsize/1024));
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
    BYTE buffer[512];   /* file copy buffer */
    FRESULT fr;     /* FatFs return code */
    UINT br = 512;
    int block = 0;
    char line[100]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */

    char path[256];
    sprintf(&path, "%s/%s", current_path, directory[current_index].fname);
    println("%s", path);
    fr = f_open(&fil, path, FA_READ);
    if (fr) {
        gui_print("Error opening file", MARKUP_ALERT);
        println("%u", (int)fr);
        return (int)fr;
    }
  
    BSP_QSPI_Init();

    while(br == sizeof buffer) { 
        f_read(&fil, buffer, sizeof buffer, &br);
        BSP_QSPI_Write(&buffer, (SDRAM_OFFSET * sequencer_channel) + (512 * block), sizeof buffer);
        block++;
    }

    println("Finished reading");
    gui_console_reset();
    file_manager_draw();
    /* Close the file */
    f_close(&fil);
}
