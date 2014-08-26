/**************************************************************************//**
* @file    drv_sdio.h
* @brief   SDIO driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_SDIO_H_
#define _DRV_SDIO_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_SDIO_SD,
    DRV_ID_SDIO_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_sdio_v[];

#endif // _DRV_SDIO_H_