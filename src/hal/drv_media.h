/**************************************************************************//**
* @file    drv_media.h
* @brief   Media driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_MEDIA_H_
#define _DRV_MEDIA_H_

//-----------------------------------------------------------------------------
enum {
    DRV_REQ_MEDIA_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_MEDIA_STATUS_GET,
    DRV_REQ_MEDIA_SECTOR_COUNT_GET,
    DRV_REQ_MEDIA_SECTOR_SIZE_GET,
    DRV_REQ_MEDIA_BLOCK_SIZE_GET,
    DRV_REQ_MEDIA_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_media_v[];

#endif // _DRV_MEDIA_H_