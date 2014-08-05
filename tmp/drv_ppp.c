/**************************************************************************//**
* @file    drv_ppp.c
* @brief   PPP driver.
* @author  
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_ppp"

//-----------------------------------------------------------------------------
/// @brief   Init PPP.
/// @return  #Status.
Status PPP_Init_(void);

extern HAL_DriverItf drv_pppx;

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_ppp_v[DRV_ID_PPP_LAST];

/*****************************************************************************/
Status PPP_Init_(void)
{
    memset(drv_ppp_v, 0x0, sizeof(drv_ppp_v));
    drv_ppp_v[DRV_ID_PPPX] = &drv_pppx;
    return S_OK;
}
