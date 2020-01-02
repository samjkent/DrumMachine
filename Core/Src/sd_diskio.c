 /* Includes ------------------------------------------------------------------*/
 #include <string.h>
 #include "ff_gen_drv.h"
 #include "stm32f769i_discovery_sd.h"
 
 /* Private typedef -----------------------------------------------------------*/
 /* Private define ------------------------------------------------------------*/
 /* Block Size in Bytes */
 #define BLOCK_SIZE                512
 
 /* Private variables ---------------------------------------------------------*/
 /* Disk status */
 static volatile DSTATUS Stat = STA_NOINIT;
 
 /* Private function prototypes -----------------------------------------------*/
 DSTATUS SD_initialize (BYTE);
 DSTATUS SD_status (BYTE);
 DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
 #if _USE_WRITE == 1
   DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
 #endif /* _USE_WRITE == 1 */
 #if _USE_IOCTL == 1
   DRESULT SD_ioctl (BYTE, BYTE, void*);
 #endif  /* _USE_IOCTL == 1 */
   
 const Diskio_drvTypeDef  SD_Driver =
 {
   SD_initialize,
   SD_status,
   SD_read, 
 #if  _USE_WRITE == 1
   SD_write,
 #endif /* _USE_WRITE == 1 */
   
 #if  _USE_IOCTL == 1
   SD_ioctl,
 #endif /* _USE_IOCTL == 1 */
 };
 
 /* Private functions ---------------------------------------------------------*/
 
 DSTATUS SD_initialize(BYTE lun)
 {
   Stat = STA_NOINIT;
   
   /* Configure the uSD device */
   if(BSP_SD_Init() == MSD_OK)
   {
     printf("BSP_SD_Init OK \r\n");
     Stat &= ~STA_NOINIT;
   }
 
   return Stat;
 }
 
 DSTATUS SD_status(BYTE lun)
 {
   Stat = STA_NOINIT;
 
   if(BSP_SD_GetCardState() == MSD_OK)
   {
     Stat &= ~STA_NOINIT;
   }
   
   return Stat;
 }
 
 DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
 {
   DRESULT res = RES_OK;
   if(BSP_SD_ReadBlocks((uint32_t*)buff, 
                        (uint64_t) (sector * BLOCK_SIZE), 
                        count,
                        HAL_MAX_DELAY
                        ) != MSD_OK)
   {
     res = RES_ERROR;
   }

   while(SD_status(0) != SD_TRANSFER_OK);

   printf("read from sector %#x \r\n", sector * BLOCK_SIZE) ;
   for(int i = 0; i < count; i++) {
    printf("%x", buff);
   }
   printf("\r\n");

   return res;
 }
 
 DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
 {
   DRESULT res = RES_OK;
   if(BSP_SD_WriteBlocks((uint32_t*)buff, 
                         (uint64_t)(sector * BLOCK_SIZE), 
                         count,
                         HAL_MAX_DELAY
                         ) != MSD_OK)
   {
     res = RES_ERROR;
   }

   while(SD_status(0) != SD_TRANSFER_OK);

   printf("write ");
   for(int i = 0; i < count; i++) {
    printf("%x", buff);
   }
   printf("\r\n");

   return res;
 }
 
 DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
 {
   DRESULT res = RES_ERROR;
   BSP_SD_CardInfo CardInfo;
   
   if (Stat & STA_NOINIT) return RES_NOTRDY;
    
   printf("block size: %d , ioctl cmd %d \r\n", BLOCK_SIZE, cmd); 
   switch (cmd)
   {
   /* Make sure that no pending write process */
   case CTRL_SYNC :
     res = RES_OK;
     break;
   
   /* Get number of sectors on the disk (DWORD) */
   case GET_SECTOR_COUNT :
     BSP_SD_GetCardInfo(&CardInfo);
     *(DWORD*)buff = CardInfo.BlockNbr;
     // *(DWORD*)buff = CardInfo.CardCapacity / BLOCK_SIZE;
     res = RES_OK;
     break;
   
   /* Get R/W sector size (WORD) */
   case GET_SECTOR_SIZE :
     printf("GET_SECTOR_SIZE \r\n"); 
     *(WORD*)buff = BLOCK_SIZE;
     res = RES_OK;
     break;
   
   /* Get erase block size in unit of sector (DWORD) */
   case GET_BLOCK_SIZE :
     printf("GET_BLOCK_SIZE \r\n"); 
     *(DWORD*)buff = BLOCK_SIZE;
     res = RES_OK;
     break;
   
   default:
     res = RES_PARERR;
   }
   
   printf("ioctl res %d \r\n", res); 
   
   return res;
 }
   
 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
