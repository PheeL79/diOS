/**************************************************************************//**
* @file    drv_crc.h
* @brief   CRC driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_CRC_H_
#define _DRV_CRC_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_CRC,
    DRV_ID_CRC_LAST
};

enum {
    DRV_REQ_CRC_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_CRC_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_crc_v[];

#endif // _DRV_CRC_H_