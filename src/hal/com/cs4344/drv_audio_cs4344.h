/**************************************************************************//**
* @file    drv_audio_cs4344.h
* @brief   CS4344 audio driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_AUDIO_CS4344_H_
#define _DRV_AUDIO_CS4344_H_

#include "os_audio.h"

//-----------------------------------------------------------------------------
enum {
    DRV_REQ_AUDIO_CS4344_UNDEF = DRV_REQ_AUDIO_LAST,
    DRV_REQ_AUDIO_CS4344_LAST
};

typedef struct {
    OS_AudioFreq        freq;
    OS_AudioVolume      volume;
} CS4344_DrvAudioArgsInit;

#endif // _DRV_AUDIO_CS4344_H_