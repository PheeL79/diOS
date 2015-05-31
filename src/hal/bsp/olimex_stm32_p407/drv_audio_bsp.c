/**************************************************************************//**
* @file    drv_audio_bsp.c
* @brief   Audio BSP driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "drv_audio_bsp.h"

#if (OS_AUDIO_ENABLED)
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
extern HAL_DriverItf drv_cs4344;
    HAL_MemSet(drv_audio_v, 0x0, sizeof(drv_audio_v));
    drv_audio_v[DRV_ID_CS4344]  = &drv_cs4344;
    return S_OK;
}

#endif //(OS_AUDIO_ENABLED)