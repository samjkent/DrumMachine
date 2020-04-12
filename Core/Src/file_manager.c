#include "file_manager.h"
#include "gui.h"
#include "ff_gen_drv.h"
#include "ff.h"

char* current_path;
uint8_t current_index;

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    println("print files: \r\n");

    res = f_mount(&SDFatFs, SDPath, 1);

    res = f_opendir(&dir, path);                           /* Open the directory */
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
    return res;
}

void attempt_fmount() {
    FRESULT retSD;
    FIL fil;            /* File object */
    UINT br, bw;         /* File read/write count */

    retSD = f_mount(&SDFatFs, SDPath, 1);

    HAL_SD_CardInfoTypeDef card_info;
    BSP_SD_GetCardInfo(&card_info);

    if(retSD == 0) {
        retSD = f_open (
            &fil,
            "test.txt",
            FA_OPEN_APPEND | FA_WRITE | FA_READ
        );
    } else {
        println("f_mount failed %d", retSD); 
    }

    /* Write a message 
    retSD = f_lseek(&fil, f_size(&fil));
    char test[] = "sam kent was here\r\n"; 
    retSD = f_write(&fil, &test, sizeof test, &bw);
    if (bw != (sizeof test)) {
        println("failed writing %d\r\n", bw);
        println("error: %d \r\n", retSD); 
    }
    */


    f_lseek(&fil, 0);
    char line[100]; /* Line buffer */
    /* Read every line and display it */
    while (f_gets(line, sizeof line, &fil)) {
        printf(line);
    }
    /* Close the file */
    retSD = f_close(&fil);
    
    if(retSD != 0) {
        println("f_close failed: %d", retSD);
    } else {
        println("file closed successfully");
    }

    /* Unregister work area 
    */
    f_mount(0, "", 0);
    
}

void file_manager_draw() {
    gui_print("File Manager", MARKUP_HEADING);
    for(uint8_t i = current_index; i < current_index + 10; i++) {
        gui_print(directory[i].fname, (!i ? MARKUP_INVERT : MARKUP_NONE));
    }
}
