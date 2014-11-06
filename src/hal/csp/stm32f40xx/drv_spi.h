/**************************************************************************//**
* @file    drv_spi.h
* @brief   SPI driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_SPI_H_
#define _DRV_SPI_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_SPI1,
    DRV_ID_SPI2,
    DRV_ID_SPI3,
    DRV_ID_SPI_LAST
};

enum {
    DRV_REQ_SPI_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_SPI_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_spi_v[];

#endif // _DRV_SPI_H_