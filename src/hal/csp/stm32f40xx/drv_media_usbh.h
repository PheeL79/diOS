/**************************************************************************//**
* @file    drv_media_usbh.h
* @brief   MEDIA USBH driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_MEDIA_USBH_H_
#define _DRV_MEDIA_USBH_H_

#include "drv_usb.h"
#include "os_driver.h"

//-----------------------------------------------------------------------------
typedef struct {
    OS_UsbItfHd usb_itf_hd;
    OS_DriverHd drv_led_fs;
} DrvMediaUsbArgsInit;

#endif // _DRV_MEDIA_USBH_H_