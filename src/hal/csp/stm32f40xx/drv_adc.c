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
Status s = S_OK;
    HAL_MemSet(drv_adc_v, 0x0, sizeof(drv_adc_v));
    drv_adc_v[DRV_ID_ADC3] = &drv_adc3;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_adc_v, drv_adc_v[0]); ++i) {
        //Ignore specific driver(s).
        if (DRV_ID_ADC3 == i) { continue; }
        if (OS_NULL != drv_adc_v[i]) {
            IF_STATUS(s = drv_adc_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s;
}

#if (OS_AUDIO_ENABLED)
//#define FILTER_IIR(val_in, state0, shift, val_out)          do {\
//                                                                static S64 state = (state0) << shift;\
//                                                                state = state - (state >> (shift)) + (val_in);\
//                                                                val_out = (S32)(state >> (shift));\
//                                                            } while (0)

#define FILTER_IIR_RESETABLE(val_in, state0, shift, val_out)    do {\
                                                                    static S64 state;\
                                                                    static Bool reset = OS_TRUE;\
                                                                    if (reset) {\
                                                                        reset = OS_FALSE;\
                                                                        state = (S64)(state0) << shift;\
                                                                    }\
                                                                    state = state - (state >> (shift)) + (val_in);\
                                                                    val_out = (S32)(state >> (shift));\
                                                                } while (0)

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
        register OS_AudioVolume volume_curr;
        register S32 volume_adc = HAL_ADC_GetValue(adc_hd_p);
        FILTER_IIR_RESETABLE(volume_adc, volume_adc, 4, volume_adc);
        volume_curr = OS_AUDIO_VOLUME_CONVERT(volume_adc);
        if (volume_curr != volume_last) {
            volume_last = volume_curr;
            const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_ADC3, OS_SIG_DRV, (OS_SignalData)volume_last);
            OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(trimmer_stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
        }
    }
}
#endif //(OS_AUDIO_ENABLED)

/******************************************************************************/
/**
  * @brief  This function handles ADC interrupt request.
*/
void ADC_IRQHandler(void);
void ADC_IRQHandler(void)
{
extern ADC_HandleTypeDef adc3_hd;
    HAL_ADC_IRQHandler(&adc3_hd);
}
