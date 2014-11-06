/**************************************************************************//**
* @file    drv_audio_bsp.c
* @brief   Audio BSP driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_audio_bsp"

//-----------------------------------------------------------------------------
/// @brief   Init audio.
/// @return  #Status.
Status AUDIO_Init_(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_audio_v[DRV_ID_AUDIO_LAST];

/*****************************************************************************/
Status AUDIO_Init_(void)
{
extern HAL_DriverItf drv_audio_cs4344;
    memset(drv_audio_v, 0x0, sizeof(drv_audio_v));
    drv_audio_v[DRV_ID_AUDIO_CS4344]    = &drv_audio_cs4344;
    return S_OK;
}