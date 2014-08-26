/**************************************************************************//**
* @file    drv_usbh.h
* @brief   USB Host driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_USBH_H_
#define _DRV_USBH_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_USBH,
    DRV_ID_USBH_MSC,
    DRV_ID_USBH_LAST
};

enum {
    DRV_REQ_USBH_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_USBH_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_usbh_v[];

#endif // _DRV_USBH_H_