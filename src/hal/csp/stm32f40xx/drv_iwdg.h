/**************************************************************************//**
* @file    drv_ppp.h
* @brief   PPP driver.
* @author
******************************************************************************/
#ifndef _DRV_IWDG_H_
#define _DRV_IWDG_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_IWDG,
    DRV_ID_IWDG_LAST
};

enum {
    DRV_REQ_IWDG_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_IWDG_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_iwdg_v[];

#endif // _DRV_IWDG_H_