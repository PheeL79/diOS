/**************************************************************************//**
* @file    drv_media.h
* @brief   MEDIA driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_MEDIA_H_
#define _DRV_MEDIA_H_

#include "os_config.h"

//-----------------------------------------------------------------------------
enum {
#if defined(OS_MEDIA_VOL_SDRAM)
    DRV_ID_MEDIA_SDRAM  = OS_MEDIA_VOL_SDRAM,
#endif
#if defined(OS_MEDIA_VOL_SDCARD)
    DRV_ID_MEDIA_SDCARD = OS_MEDIA_VOL_SDCARD,
#endif
#if defined(OS_MEDIA_VOL_USBH_FS)
    DRV_ID_MEDIA_USBH_FS = OS_MEDIA_VOL_USBH_FS,
#endif
#if defined(OS_MEDIA_VOL_USBH_HS)
    DRV_ID_MEDIA_USBH_HS = OS_MEDIA_VOL_USBH_HS,
#endif
    DRV_ID_MEDIA_LAST
};

enum {
    DRV_REQ_MEDIA_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_MEDIA_STATUS_GET,
    DRV_REQ_MEDIA_SECTOR_COUNT_GET,
    DRV_REQ_MEDIA_SECTOR_SIZE_GET,
    DRV_REQ_MEDIA_BLOCK_SIZE_GET,
    DRV_REQ_MEDIA_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_media_v[];

#endif // _DRV_MEDIA_H_