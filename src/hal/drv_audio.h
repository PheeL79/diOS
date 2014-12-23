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
    DRV_REQ_AUDIO_INPUT_SETUP,
    DRV_REQ_AUDIO_INPUT_BITS_GET,
    DRV_REQ_AUDIO_INPUT_BITS_SET,
    DRV_REQ_AUDIO_INPUT_FREQUENCY_GET,
    DRV_REQ_AUDIO_INPUT_FREQUENCY_SET,
    DRV_REQ_AUDIO_OUTPUT_SETUP,
    DRV_REQ_AUDIO_OUTPUT_BITS_GET,
    DRV_REQ_AUDIO_OUTPUT_BITS_SET,
    DRV_REQ_AUDIO_OUTPUT_FREQUENCY_GET,
    DRV_REQ_AUDIO_OUTPUT_FREQUENCY_SET,
    DRV_REQ_AUDIO_LAST
};

typedef struct {
    U8*  data_p;
    Size size;
} DrvAudioPlayArgs;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_audio_v[];

#endif // _DRV_AUDIO_H_