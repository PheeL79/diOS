/**************************************************************************//**
* @file    drv_crc.h
* @brief   CRC driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_CRC_H_
#define _DRV_CRC_H_

#if (HAL_CRC_ENABLED)

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

#endif //(HAL_CRC_ENABLED)

#endif // _DRV_CRC_H_