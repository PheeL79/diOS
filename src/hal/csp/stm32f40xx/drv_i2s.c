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

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_i2s_v[DRV_ID_I2S_LAST];
OS_QueueHd audio_stdin_qhd;

/*****************************************************************************/
Status I2S_Init_(void)
{
//extern HAL_DriverItf drv_i2s3;
    HAL_MemSet(drv_i2s_v, 0x0, sizeof(drv_i2s_v));
//    drv_i2s_v[DRV_ID_I2S3] = &drv_i2s3;
    return S_OK;
}

/******************************************************************************/
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if (CS4344_I2Sx == hi2s->Instance) {
        const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_AUDIO_CS4344, OS_SIG_AUDIO_ERROR, 0);
        if (1 == OS_ISR_SignalSend(audio_stdin_qhd, signal, OS_MSG_PRIO_NORMAL)) {
            OS_ContextSwitchForce();
        }
    }
}
