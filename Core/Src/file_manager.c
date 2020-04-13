#include "file_manager.h"
#include "gui.h"
#include "ff_gen_drv.h"
#include "ff.h"

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
    FIL fil;            /* File object */
    UINT br, bw;         /* File read/write count */

    retSD = f_mount(&SDFatFs, SDPath, 1);

}

void file_manager_draw() {
    gui_console_reset();
    gui_print("File Manager", MARKUP_HEADING);
    for(uint8_t i = current_index; (i < current_index + 15) && (i < (max_index+1)); i++) {
        if(directory[i].fname == 0) break;
        char fname[30];
        sprintf(&fname, "%s", directory[i].fname);
        if(directory[i].fattrib == AM_DIR) {
            sprintf(&fname, "%s/", fname);
        }
        gui_print(fname, (i == current_index ? MARKUP_INVERT : MARKUP_NONE));
    }
}

void file_manager_select() {
    if(directory[current_index].fattrib == AM_DIR) {
        sprintf(&current_path, "%s/%s", current_path, directory[current_index].fname);
        scan_files();
        current_index = 0;
    }
}

void file_manager_index_inc() {
    if(current_index < (max_index - 1)) {
        current_index++;
    }
}

void file_manager_index_dec() {
    if(current_index > 0) {
        current_index--;
    }
}
