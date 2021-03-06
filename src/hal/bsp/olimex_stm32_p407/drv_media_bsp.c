/**************************************************************************//**
* @file    drv_media_bsp.c
* @brief   Media driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

#if (OS_FILE_SYSTEM_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_media_bsp"

//-----------------------------------------------------------------------------
/// @brief   Init media.
/// @return  #Status.
Status MEDIA_Init_(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_media_v[DRV_ID_MEDIA_LAST];

/*****************************************************************************/
Status MEDIA_Init_(void)
{
    HAL_MemSet(drv_media_v, 0x0, sizeof(drv_media_v));
#if defined(OS_MEDIA_VOL_SDRAM)
extern HAL_DriverItf drv_media_sdram;
    drv_media_v[DRV_ID_MEDIA_SDRAM]     = &drv_media_sdram;
#endif
#if defined(OS_MEDIA_VOL_SDCARD)
extern HAL_DriverItf drv_media_sdcard;
    drv_media_v[DRV_ID_MEDIA_SDCARD]    = &drv_media_sdcard;
#endif
#if defined(OS_MEDIA_VOL_USBH_FS)
extern HAL_DriverItf drv_media_usbh_fs;
    drv_media_v[DRV_ID_MEDIA_USBH_FS]   = &drv_media_usbh_fs;
#endif
#if defined(OS_MEDIA_VOL_USBH_HS)
extern HAL_DriverItf drv_media_usbh_hs;
    drv_media_v[DRV_ID_MEDIA_USBH_HS]   = &drv_media_usbh_hs;
#endif
    return S_OK;
}

#endif //(OS_FILE_SYSTEM_ENABLED)