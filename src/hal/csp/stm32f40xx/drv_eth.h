/**************************************************************************//**
* @file    drv_eth.h
* @brief   ETH driver.
* @author
******************************************************************************/
#ifndef _DRV_ETH_H_
#define _DRV_ETH_H_

//-----------------------------------------------------------------------------
extern HAL_DriverItf                    drv_ks8721bl;
#define DRV_ETH0_PHY                    drv_ks8721bl
#define ETH0_PHY_ADDR                   1

enum {
    DRV_ID_ETH0,
    DRV_ID_ETH_LAST
};

enum {
    DRV_REQ_ETH_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_ETH_LINK_STATUS_GET,
    DRV_REQ_ETH_SETUP,
    DRV_REQ_ETH_LAST
};

//typedef OS_NetworkItfInitArgs ETH_DrvArgsInit;
//typedef OS_NetworkItfOpenArgs ETH_DrvArgsOpen;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_eth_v[];

#include "com/ks8721bl/drv_ks8721bl.h"

#endif // _DRV_ETH_H_