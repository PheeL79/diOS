/**************************************************************************//**
* @file    drv_audio.h
* @brief   Audio driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_AUDIO_H_
#define _DRV_AUDIO_H_

#include "hal.h"

//-----------------------------------------------------------------------------
enum {
    DRV_REQ_AUDIO_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_AUDIO_PLAY,
    DRV_REQ_AUDIO_STOP,
    DRV_REQ_AUDIO_PAUSE,
    DRV_REQ_AUDIO_RESUME,
    DRV_REQ_AUDIO_SEEK,
    DRV_REQ_AUDIO_MUTE_SET,
    DRV_REQ_AUDIO_VOLUME_GET,
    DRV_REQ_AUDIO_VOLUME_SET,
    DRV_REQ_AUDIO_FREQUENCY_GET,
    DRV_REQ_AUDIO_FREQUENCY_SET,
    DRV_REQ_AUDIO_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_audio_v[];

#endif // _DRV_AUDIO_H_