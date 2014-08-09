/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "os_file_system.h"

extern OS_DriverHd fs_media_dhd_v[];

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
    IF_STATUS_OK(OS_FileSystemMediaInit(OS_FileSystemMediaByVolumeGet(pdrv))) { return 0; }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
    //TODO(A. Filyanov) Media disk status.
    return 0;//STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Sector address (LBA) */
    UINT count		/* Number of sectors to read (1..128) */
)
{
DRESULT res;
    IF_STATUS_OK(OS_DriverRead(fs_media_dhd_v[pdrv], buff, count, &sector)) {
        res = RES_OK;
    } else {
        res = RES_ERROR;
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber (0..) */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Sector address (LBA) */
    UINT count			/* Number of sectors to write (1..128) */
)
{
DRESULT res;
    IF_STATUS_OK(OS_DriverWrite(fs_media_dhd_v[pdrv], (void *)buff, count, &sector)) {
        res = RES_OK;
    } else {
        res = RES_ERROR;
    }
    return res;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
DRESULT res;
    IF_STATUS_OK(OS_DriverIoCtl(fs_media_dhd_v[pdrv], cmd, buff)) {
        res = RES_OK;
    } else {
        res = RES_ERROR;
    }
    return res;
}
#endif
