/**************************************************************************//**
* @file    drv_i2s.c
* @brief   I2S driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "drv_audio_bsp.h"
#include "os_supervise.h"
#include "os_audio.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_i2s"

//-----------------------------------------------------------------------------
/// @brief   Init I2S.
/// @return  #Status.
Status I2S_Init_(void);

#if (OS_AUDIO_ENABLED)
extern OS_ISR_AudioDeviceCallback CS4344_ISR_DrvAudioDeviceCallback;
extern OS_AudioDeviceCallbackArgs cs4344_callback_args;
#endif //(OS_AUDIO_ENABLED)

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_i2s_v[DRV_ID_I2S_LAST];

/*****************************************************************************/
Status I2S_Init_(void)
{
//extern HAL_DriverItf drv_i2s3;
    HAL_MemSet(drv_i2s_v, 0x0, sizeof(drv_i2s_v));
//    drv_i2s_v[DRV_ID_I2S3] = &drv_i2s3;
    return S_OK;
}

#if (OS_AUDIO_ENABLED)
/******************************************************************************/
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (CS4344_I2Sx == hi2s->Instance) {
        cs4344_callback_args.signal_id = OS_SIG_AUDIO_TX_COMPLETE_HALF;
        CS4344_ISR_DrvAudioDeviceCallback(&cs4344_callback_args);
    }
}

/******************************************************************************/
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (CS4344_I2Sx == hi2s->Instance) {
        cs4344_callback_args.signal_id = OS_SIG_AUDIO_TX_COMPLETE;
        CS4344_ISR_DrvAudioDeviceCallback(&cs4344_callback_args);
    }
}

/******************************************************************************/
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if (CS4344_I2Sx == hi2s->Instance) {
        cs4344_callback_args.signal_id = OS_SIG_AUDIO_ERROR;
        CS4344_ISR_DrvAudioDeviceCallback(&cs4344_callback_args);
    }
}
#endif //(OS_AUDIO_ENABLED)