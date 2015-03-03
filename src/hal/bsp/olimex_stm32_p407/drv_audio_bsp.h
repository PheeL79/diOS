/**************************************************************************//**
* @file    drv_audio_bsp.h
* @brief   Audio BSP driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_AUDIO_BSP_H_
#define _DRV_AUDIO_BSP_H_

#include "os_config.h"
#include "com/cs4344/drv_cs4344.h"

//-----------------------------------------------------------------------------
enum {
#if defined(OS_AUDIO_DEV_CS4344)
    DRV_ID_AUDIO_CS4344  = OS_AUDIO_DEV_CS4344,
#endif
    DRV_ID_AUDIO_LAST
};

#endif // _DRV_AUDIO_BSP_H_