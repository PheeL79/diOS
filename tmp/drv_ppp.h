/**************************************************************************//**
* @file    drv_ppp.h
* @brief   PPP driver.
* @author  
******************************************************************************/
#ifndef _DRV_PPP_H_
#define _DRV_PPP_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_PPPX,
    DRV_ID_PPP_LAST
};

enum {
    DRV_REQ_PPP_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_PPP_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_ppp_v[];

#endif // _DRV_PPP_H_