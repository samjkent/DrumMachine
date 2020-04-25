#include "ff_gen_drv.h"

#define MAX_DIRECTORY_FILES 256

FATFS SDFatFs;
FIL file;
extern Diskio_drvTypeDef SD_Driver;
char SDPath[4];

uint8_t fm_directory_index;
FILINFO directory[MAX_DIRECTORY_FILES];

void file_manager_scan_current_directory();
void file_manager_select();


