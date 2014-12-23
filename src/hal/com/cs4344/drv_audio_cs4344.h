/**************************************************************************//**
* @file    drv_audio_cs4344.h
* @brief   CS4344 audio driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_AUDIO_CS4344_H_
#define _DRV_AUDIO_CS4344_H_

#include "drv_audio.h"
#include "os_audio.h"

//-----------------------------------------------------------------------------
#define CS4344_I2Sx                         SPI3

enum {
    DRV_REQ_AUDIO_CS4344_UNDEF = DRV_REQ_AUDIO_LAST,
    DRV_REQ_AUDIO_CS4344_LAST
};

typedef OS_AudioDeviceIoSetupArgs CS4344_DrvAudioArgsInit;
typedef OS_AudioDeviceArgsOpen CS4344_DrvAudioArgsOpen;

extern const OS_AudioDeviceCaps cs4344_caps;

#endif // _DRV_AUDIO_CS4344_H_