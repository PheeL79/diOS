/**************************************************************************//**
* @file    drv_i2s.h
* @brief   I2S driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_I2S_H_
#define _DRV_I2S_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_I2S1,
    DRV_ID_I2S2,
    DRV_ID_I2S3,
    DRV_ID_I2S_LAST
};

enum {
    DRV_REQ_I2S_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_I2S_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_i2s_v[];

#endif // _DRV_I2S_H_