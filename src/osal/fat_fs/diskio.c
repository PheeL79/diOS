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
//#include "usbdisk.h"	/* Example: USB drive control */
//#include "atadrive.h"	/* Example: ATA drive control */
#include "sdcard.h"		/* Example: MMC/SDC contorl */
#include "common.h"
#include "os_driver.h"

/* Definitions of physical drive number for each media */
#define ATA		0
#define MMC		1
#define USB		2

static OS_DriverHd drv_sdio_sd;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
    DSTATUS stat;
    int result;

    switch (pdrv) {
    case ATA :
        //result = ATA_disk_initialize();

        // translate the reslut code here
        stat = 0;
        return stat;

    case MMC :
        result = MMC_disk_initialize(&drv_sdio_sd);

        // translate the reslut code here
        stat = (DSTATUS)result;
        return stat;

    case USB :
        //result = USB_disk_initialize();

        // translate the reslut code here

        return stat;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
    DSTATUS stat;
    int result;

    switch (pdrv) {
    case ATA :
        //result = ATA_disk_status();

        // translate the reslut code here
        stat = 0;
        return stat;

    case MMC :
        result = MMC_disk_status();

        // translate the reslut code here
        stat = (DSTATUS)result;
        return stat;

    case USB :
        //result = USB_disk_status();

        // translate the reslut code here

        return stat;
    }
    return STA_NOINIT;
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
    int result;

    switch (pdrv) {
    case ATA :
        // translate the arguments here

        //result = ATA_disk_read(buff, sector, count);

        // translate the reslut code here
        res = RES_ERROR;
        return res;

    case MMC : {
        // translate the arguments here
        Status s = OS_DriverRead(drv_sdio_sd, buff, count, &sector);

        // translate the reslut code here
        IF_STATUS(s) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        }
        return res;

    case USB :
        // translate the arguments here

        //result = USB_disk_read(buff, sector, count);

        // translate the reslut code here

        return res;
    }
    return RES_PARERR;
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
    int result;

    switch (pdrv) {
    case ATA :
        // translate the arguments here

        //result = ATA_disk_write(buff, sector, count);

        // translate the reslut code here
        res = RES_ERROR;
        return res;

    case MMC : {
        // translate the arguments here
        Status s = OS_DriverWrite(drv_sdio_sd, (void *)buff, count, &sector);

        // translate the reslut code here
        IF_STATUS(s) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        }
        return res;

    case USB :
        // translate the arguments here

        //result = USB_disk_write(buff, sector, count);

        // translate the reslut code here

        return res;
    }
    return RES_PARERR;
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
//	int result;

    switch (pdrv) {
    case ATA :
        // pre-process here

        //result = ATA_disk_ioctl(cmd, buff);

        // post-process here
        res = RES_ERROR;
        return res;

    case MMC : {
        // translate the arguments here
        Status s = OS_DriverIoCtl(drv_sdio_sd, cmd, buff);

        // translate the reslut code here
        IF_STATUS(s) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        }
        return res;

    case USB :
        // pre-process here

        //result = USB_disk_ioctl(cmd, buff);

        // post-process here

        return res;
    }
    return RES_PARERR;
}
#endif
