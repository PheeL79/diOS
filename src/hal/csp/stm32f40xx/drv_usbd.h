/**************************************************************************//**
* @file    drv_usbd.h
* @brief   USB Device driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_USBD_H_
#define _DRV_USBD_H_

//-----------------------------------------------------------------------------
enum {
    DRV_REQ_USBD_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_USBD_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_usbd_v[];

#endif // _DRV_USBD_H_