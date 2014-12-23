/***************************************************************************//**
* @file    os_audio.h
* @brief   OS Audio.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_AUDIO_H_
#define _OS_AUDIO_H_

#include "os_common.h"
#include "os_signal.h"
#include "os_mutex.h"
#include "os_driver.h"

#if (1 == OS_AUDIO_ENABLED)
/**
* \defgroup OS_Audio OS_Audio
* @{
*/
//------------------------------------------------------------------------------
#define OS_AUDIO_IN_DEVICE_DEFAULT          OS_NULL
#define OS_AUDIO_OUT_DEVICE_DEFAULT         OS_NULL

#define OS_AUDIO_VOLUME_MIN                 0
#define OS_AUDIO_VOLUME_MAX                 100

#define OS_AUDIO_VOLUME_CONVERT(volume)     ((OS_AudioVolume)(((Float)volume / ((Float)U8_MAX * 1.0)) * ((Float)OS_AUDIO_VOLUME_MAX * 1.0)))

//------------------------------------------------------------------------------
typedef void*   OS_AudioDeviceHd;
typedef S8      OS_AudioVolume;
typedef S8      OS_AudioSampleBits;
typedef S32     OS_AudioSampleRate;
typedef OS_AudioSampleRate OS_AudioFreq;
typedef OS_AudioSampleBits OS_AudioBits;

enum {
    OS_SIG_AUDIO_TX_COMPLETE = OS_SIG_APP,
    OS_SIG_AUDIO_TX_COMPLETE_HALF,
    OS_SIG_AUDIO_ERROR,
    OS_SIG_AUDIO_LAST
};

enum {
    OS_AUDIO_CHANNELS_UNDEF,
    OS_AUDIO_CHANNELS_MONO,
    OS_AUDIO_CHANNELS_STEREO,
    OS_AUDIO_CHANNELS_LAST
};
typedef S8 OS_AudioChannels;

enum {
    OS_AUDIO_DEVICE_IN_NONE,
    OS_AUDIO_DEVICE_IN_MICROPHONE,
    OS_AUDIO_DEVICE_LAST
};
typedef S8 OS_AudioDeviceIn;

enum {
    OS_AUDIO_DEVICE_OUT_NONE,
    OS_AUDIO_DEVICE_OUT_ALL,
    OS_AUDIO_DEVICE_OUT_AUTO,
    OS_AUDIO_DEVICE_OUT_SPEAKER,
    OS_AUDIO_DEVICE_OUT_HEADPHONE,
    OS_AUDIO_DEVICE_OUT_LAST
};
typedef S8 OS_AudioDeviceOut;

enum {
    OS_AUDIO_DMA_MODE_UNDEF,
    OS_AUDIO_DMA_MODE_NORMAL,
    OS_AUDIO_DMA_MODE_CIRCULAR,
    OS_AUDIO_DMA_MODE_LAST,
};
typedef S8 OS_AudioDmaMode;

typedef struct {
    OS_AudioSampleRate  sample_rate;
    OS_AudioSampleBits  sample_bits;
    OS_AudioChannels    channels;
} OS_AudioInfo;

typedef struct {
    const OS_AudioSampleRate*   sample_rates_vp;
    const OS_AudioSampleBits*   sample_bits_vp;
    OS_AudioChannels            channels;
    OS_AudioDeviceIn            in;
} OS_AudioDeviceCapsInput;

typedef struct {
    const OS_AudioSampleRate*   sample_rates_vp;
    const OS_AudioSampleBits*   sample_bits_vp;
    OS_AudioChannels            channels;
    OS_AudioDeviceOut           out;
} OS_AudioDeviceCapsOutput;

typedef struct {
    const OS_AudioDeviceCapsInput*  input_p;
    const OS_AudioDeviceCapsOutput* output_p;
//    Bool                            is_single;
} OS_AudioDeviceCaps;

typedef struct {
    Str                 name[OS_AUDIO_DEVICE_NAME_LEN];
    OS_DriverConfig*    drv_cfg_p;
    OS_AudioDeviceCaps  caps;
} OS_AudioDeviceConfig;

typedef struct {
    S8 reserved;
} OS_AudioDeviceStats;

typedef struct {
    OS_AudioInfo    info;
    OS_AudioVolume  volume;
    OS_AudioDmaMode dma_mode;
} OS_AudioDeviceIoSetupArgs;

typedef struct {
    OS_QueueHd  slot_qhd;
    OS_SignalId signal_id;
} OS_AudioDeviceCallbackArgs;

typedef void (*OS_ISR_AudioDeviceCallback)(OS_AudioDeviceCallbackArgs* args_p);

typedef struct {
    OS_QueueHd                  slot_qhd;
    OS_ISR_AudioDeviceCallback  isr_callback_func;
} OS_AudioDeviceArgsOpen;

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

/// @brief      Get the system default audio device.
/// @param[in]  dir             Audio direction.
/// @return     Device handle.
OS_AudioDeviceHd OS_AudioDeviceDefaultGet(const Direction dir);

/// @brief      Set the system default audio device.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  dir             Audio direction.
/// @return     #Status.
Status          OS_AudioDeviceDefaultSet(const OS_AudioDeviceHd dev_hd, const Direction dir);

/// @brief      Get the audio device current status.
/// @param[in]  dev_hd          Device handle.
/// @return     #Status.
Status          OS_AudioDeviceStatusGet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get the audio device capabilities.
/// @param[in]  dev_hd          Device handle.
/// @param[out] caps_p          Capabilities.
/// @return     #Status.
Status          OS_AudioDeviceCapsGet(const OS_AudioDeviceHd dev_hd, OS_AudioDeviceCaps* caps_p);

/// @brief      Setup I/O for the audio device.
/// @param[in]  dev_hd          Device handle.
/// @param[in]  info            Audio info.
/// @param[in]  dir             Audio direction.
/// @return     #Status.
Status          OS_AudioDeviceIoSetup(const OS_AudioDeviceHd dev_hd, const OS_AudioDeviceIoSetupArgs* args_p, const Direction dir);

OS_AudioSampleRate OS_AudioDeviceSampleRateGet(const OS_AudioDeviceHd dev_hd, const Direction dir);

Status          OS_AudioDeviceSampleRateSet(const OS_AudioDeviceHd dev_hd, const OS_AudioSampleRate sample_rate,
                                            const Direction dir);

/// @brief      Get the audio device name.
/// @param[in]  dev_hd          Device handle.
/// @return     Volume name.
ConstStrP       OS_AudioDeviceNameGet(const OS_AudioDeviceHd dev_hd);

/// @brief      Get audio device handle by it's name.
/// @param[in]  name_p          Device volume name.
/// @return     Device handle.
OS_AudioDeviceHd OS_AudioDeviceByNameGet(ConstStrP name_p);

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
Status          OS_AudioPlay(const OS_AudioDeviceHd dev_hd, void* data_p, Size size);

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
Status          OS_AudioSeek(const OS_AudioDeviceHd dev_hd, const Size offset);

/**@}*/ //OS_Audio

#endif // (1 == OS_AUDIO_ENABLED)

#endif // _OS_AUDIO_H_
