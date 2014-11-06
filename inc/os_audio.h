/***************************************************************************//**
* @file    os_audio.h
* @brief   OS Audio.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_AUDIO_H_
#define _OS_AUDIO_H_

#include "os_common.h"
#include "os_driver.h"

#if (1 == OS_AUDIO_ENABLED)
/**
* \defgroup OS_Audio OS_Audio
* @{
*/
//------------------------------------------------------------------------------
#define OS_AUDIO_VOLUME_MIN                 0
#define OS_AUDIO_VOLUME_MAX                 100
#define OS_AUDIO_VOLUME_STEP                BIT_MASK(2)

#define OS_AUDIO_VOLUME_CONVERT(volume)     ((OS_AudioVolume)(((FLT)volume / ((FLT)U8_MAX * 1.0)) * ((FLT)OS_AUDIO_VOLUME_MAX * 1.0)))

//------------------------------------------------------------------------------
typedef void*   OS_AudioDeviceHd;
typedef U8      OS_AudioVolume;
typedef S8      OS_AudioSampleBits;
typedef S32     OS_AudioSampleRate;
typedef OS_AudioSampleRate OS_AudioFreq;

enum {
    OS_AUDIO_IO_UNDEF,
    OS_AUDIO_IO_MONO,
    OS_AUDIO_IO_STEREO,
    OS_AUDIO_IO_LAST
};
typedef S8      OS_AudioModeIo;

enum {
    OS_AUDIO_DEVICE_IN_MICROPHONE,
};
typedef S8      OS_AudioDeviceIn;

enum {
    OS_AUDIO_DEVICE_OUT_ALL,
    OS_AUDIO_DEVICE_OUT_AUTO,
    OS_AUDIO_DEVICE_OUT_SPEAKER,
    OS_AUDIO_DEVICE_OUT_HEADPHONE
};
typedef S8      OS_AudioDeviceOut;

typedef struct {
//    OS_AudioSampleRate  sample_rate_v[0];
    OS_AudioSampleBits  sample_bits;
    OS_AudioModeIo      mode_io;
    Direction           dir;
} OS_AudioDeviceCaps;

typedef struct {
    Str             name[OS_AUDIO_DEVICE_NAME_LEN];
    OS_DriverConfig*drv_cfg_p;
//    OS_AudioDeviceCaps caps;
} OS_AudioDeviceConfig;

typedef struct {
    S8    reserved;
} OS_AudioDeviceStats;

//------------------------------------------------------------------------------
/// @brief      Init the audio system.
/// @return     #Status.
Status          OS_AudioInit(void);

/// @brief      Deinit the audio system.
/// @return     #Status.
Status          OS_AudioDeInit(void);

/// @brief      Create the audio device.
/// @param[in]  cfg_p           Device config.
/// @param[out] dev_hd_p        Device handle.
/// @return     #Status.
Status          OS_AudioDeviceCreate(const OS_AudioDeviceConfig* cfg_p, OS_AudioDeviceHd* dev_hd_p);

/// @brief      Delete the audio device.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceDelete(const OS_AudioDeviceHd dev_hd);

/// @brief      Init the audio device.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  args_p          Device driver init arguments.
/// @return     #Status.
Status          OS_AudioDeviceInit(const OS_AudioDeviceHd dev_hd, void* args_p);

/// @brief      Deinit the audio device.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceDeInit(const OS_AudioDeviceHd dev_hd);

/// @brief      Open the audio device.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  args_p          Device driver open arguments.
/// @return     #Status.
Status          OS_AudioDeviceOpen(const OS_AudioDeviceHd dev_hd, void* args_p);

/// @brief      Close the audio device.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceClose(const OS_AudioDeviceHd dev_hd);

/// @brief      Get the current system audio device.
/// @return     Device handle.
OS_AudioDeviceHd OS_AudioDeviceCurrentGet(void);

/// @brief      Set the current system audio device.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceCurrentSet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get the audio device current status.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceStatusGet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get the audio device name.
/// @param[in]  dev_hd          Device handle.
/// @return     Volume name.
ConstStrPtr     OS_AudioDeviceNameGet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get audio device handle by it's name.
/// @param[in]  name_p          Device volume name.
/// @return     Device handle.
OS_AudioDeviceHd OS_AudioDeviceByNameGet(ConstStrPtr name_p);

/// @brief      Get the next audio device.
/// @param[in]  dev_hd          Device handle.
/// @return     Device handle.
OS_AudioDeviceHd OS_AudioDeviceNextGet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get the audio device volume.
/// @param[in]  dev_hd          Device handle.
/// @param[out] volume_p        Volume(0<=%=>100).
/// @return     #Status.
Status          OS_AudioVolumeGet(const OS_AudioDeviceHd dev_hd, OS_AudioVolume* volume_p);

/// @brief      Set the audio device volume.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  volume          Volume(0<=%=>100).
/// @return     #Status.
Status          OS_AudioVolumeSet(const OS_AudioDeviceHd dev_hd, const OS_AudioVolume volume);

/// @brief      Set mute state for the audio device.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  state           Mute state.
/// @return     #Status.
Status          OS_AudioMuteSet(const OS_AudioDeviceHd dev_hd, const State state);

/// @brief      Play audio buffer.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  data_p          Audio buffer data.
/// @param[in]  size            Buffer data size.
/// @return     #Status.
Status          OS_AudioPlay(const OS_AudioDeviceHd dev_hd, U8* data_p, SIZE size);

/// @brief      Stop audio device playback.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioStop(const OS_AudioDeviceHd dev_hd);

/// @brief      Pause audio device playback.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioPause(const OS_AudioDeviceHd dev_hd);

/// @brief      Resume audio device playback.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioResume(const OS_AudioDeviceHd dev_hd);

/// @brief      Seek through audio device playback.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioSeek(const OS_AudioDeviceHd dev_hd, const SIZE offset);

/**@}*/ //OS_Audio

#endif // (1 == OS_AUDIO_ENABLED)

#endif // _OS_AUDIO_H_
