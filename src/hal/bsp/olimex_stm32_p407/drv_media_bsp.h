/**************************************************************************//**
* @file    drv_media_bsp.h
* @brief   Media BSP driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_MEDIA_BSP_H_
#define _DRV_MEDIA_BSP_H_

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

#endif // _DRV_MEDIA_BSP_H_