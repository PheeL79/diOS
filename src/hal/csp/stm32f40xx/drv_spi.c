/**************************************************************************//**
* @file    drv_spi.c
* @brief   SPI driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_spi"

//-----------------------------------------------------------------------------
/// @brief   Init SPI.
/// @return  #Status.
Status SPI_Init_(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_spi_v[DRV_ID_SPI_LAST];

/*****************************************************************************/
Status SPI_Init_(void)
{
//extern HAL_DriverItf drv_spi3;
Status s = S_OK;
    HAL_MemSet(drv_spi_v, 0x0, sizeof(drv_spi_v));
//    drv_spi_v[DRV_ID_SPI3] = &drv_spi3;
    return s;
}
