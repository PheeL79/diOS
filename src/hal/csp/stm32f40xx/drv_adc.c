/**************************************************************************//**
* @file    drv_adc.c
* @brief   ADC driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_supervise.h"
#include "os_task.h"
#include "os_signal.h"
#include "os_audio.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_adc"

//-----------------------------------------------------------------------------
/// @brief   Init ADC.
/// @return  #Status.
Status ADC_Init_(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_adc_v[DRV_ID_ADC_LAST];

/*****************************************************************************/
Status ADC_Init_(void)
{
extern HAL_DriverItf drv_adc3;
Status s = S_UNDEF;
    HAL_MemSet(drv_adc_v, 0x0, sizeof(drv_adc_v));
    drv_adc_v[DRV_ID_ADC3] = &drv_adc3;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_adc_v, drv_adc_v[0]); ++i) {
        //Ignore specific driver(s).
        if (DRV_ID_ADC3 == i) { continue; }
        if (OS_NULL != drv_adc_v[i]) {
            IF_STATUS(s = drv_adc_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s = S_OK;
}

/*****************************************************************************/
static U8 MovingAverage16s8b(const U8 value);
U8 MovingAverage16s8b(const U8 value)
{
static U8 buf[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};
static U8 buf_idx = 0;
U16 buf_sum = 0;
    buf[buf_idx] = value;
    ++buf_idx;
    buf_idx &= (sizeof(buf) - 1);
    for (register U8 i = 0; i < sizeof(buf); ++i) {
        buf_sum += buf[i];
    }
    return (buf_sum >> 4);
}

#if (OS_AUDIO_ENABLED)
/*****************************************************************************/
/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc_hd_p)
{
extern OS_QueueHd trimmer_stdin_qhd;
    if (ADC3 == adc_hd_p->Instance) {
        static   OS_AudioVolume volume_last = OS_AUDIO_VOLUME_MIN;
        register OS_AudioVolume volume_curr = OS_AUDIO_VOLUME_CONVERT(HAL_ADC_GetValue(adc_hd_p));
        //First pass.
        if (volume_curr != volume_last) {
            volume_curr = MovingAverage16s8b((U8)volume_curr);
            //Second pass.
            if (volume_curr != volume_last) {
                volume_last = volume_curr;
                const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_ADC3, OS_SIG_DRV, (OS_SignalData)volume_last);
                OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(trimmer_stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
            }
        }
    }
}
#endif //(OS_AUDIO_ENABLED)
