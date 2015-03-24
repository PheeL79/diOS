/***************************************************************************//**
* @file    os_task_audio.c
* @brief   Audio daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include "drv_audio_bsp.h"
#include "os_debug.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_mailbox.h"
#include "os_audio.h"
#include "os_task_audio.h"
#include "osal.h"

#if (OS_AUDIO_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "audio_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd         drv_trimmer_hd;
    OS_AudioDeviceHd    audio_dev_hd;
} TaskStorage;

//-----------------------------------------------------------------------------
static void ISR_DrvAudioDeviceCallback(OS_AudioDeviceCallbackArgs* args_p);

//-----------------------------------------------------------------------------
const OS_TaskConfig task_audio_cfg = {
    .name           = OS_DAEMON_NAME_AUDIO,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 3,
    .prio_init      = OS_TASK_PRIO_AUDIO,
    .prio_power     = OS_TASK_PRIO_PWR_AUDIO,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN,
    .stdin_len      = OS_STDIN_LEN
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
    {
        const OS_DriverConfig drv_cfg = {
            .name       = "TRIMMER",
            .itf_p      = &drv_trimmer,
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&tstor_p->drv_trimmer_hd)) { return s; }
    }
    {
        OS_DriverConfig drv_cfg = {
            .name       = "A_CS4344",
            .itf_p      = drv_audio_v[DRV_ID_AUDIO_CS4344],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_AudioDeviceConfig audio_dev_cfg = {
            .name       = "CS4344",
            .drv_cfg_p  = &drv_cfg,
            .caps       = cs4344_caps
        };
        IF_STATUS(s = OS_AudioDeviceCreate(&audio_dev_cfg, &(tstor_p->audio_dev_hd))) { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
OS_Message* msg_p;
OS_TaskHd this_thd = OS_TaskGet();
Status s = S_UNDEF;

	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_DRV:
                        {
                        ConstStrP env_var_str_p = "volume";
                        const OS_AudioVolume volume = (OS_AudioVolume)OS_SignalDataGet(msg_p);
                        Str volume_str[4];
                            if (0 > OS_SNPrintF(volume_str, sizeof(volume_str), "%u", volume)) {
                                OS_LOG_S(D_WARNING, S_INVALID_VALUE);
                            }
                            IF_STATUS(s = OS_EnvVariableSet(env_var_str_p, volume_str, OS_NULL)) {
                                OS_LOG_S(D_WARNING, s);
                            }
                        }
                        break;
                    default:
                        OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                        break;
                }
            } else {
                switch (msg_p->id) {
                    default:
                        OS_LOG_S(D_DEBUG, S_UNDEF_MSG);
                        break;
                }
                OS_MessageDelete(msg_p); // free message allocated memory
            }
        }
    }
}

/******************************************************************************/
Status OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;

    switch (state) {
        case PWR_STARTUP: {
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            }
            break;
        case PWR_OFF:
        case PWR_STOP:
        case PWR_SHUTDOWN:
            IF_STATUS(s = OS_AudioDeviceClose(tstor_p->audio_dev_hd))       { goto error; }
            IF_STATUS(s = OS_AudioDeviceDeInit(tstor_p->audio_dev_hd))      { goto error; }
            IF_STATUS(s = OS_DriverDeInit(tstor_p->drv_trimmer_hd, OS_NULL)){ goto error; }
            break;
        case PWR_ON:
            {
            const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
                IF_OK(s = OS_DriverInit(tstor_p->drv_trimmer_hd, (void*)&stdin_qhd)) {
                    IF_STATUS(s = OS_DriverOpen(tstor_p->drv_trimmer_hd, OS_NULL)) {
                    }
                } else {
                    s = (S_INIT == s) ? S_OK : s;
                }
                const CS4344_DrvAudioArgsInit drv_args = {
                    .info.sample_rate   = OS_AUDIO_OUT_SAMPLE_RATE_DEFAULT,
                    .info.sample_bits   = OS_AUDIO_OUT_SAMPLE_BITS_DEFAULT,
                    .info.channels      = OS_AUDIO_OUT_CHANNELS_DEFAULT,
                    .volume             = OS_AUDIO_OUT_VOLUME_DEFAULT,
                    .dma_mode           = OS_AUDIO_OUT_DMA_MODE_DEFAULT
                };
                IF_STATUS(s = OS_AudioDeviceInit(tstor_p->audio_dev_hd, (void*)&drv_args)) { goto error; }
                const OS_AudioDeviceArgsOpen audio_dev_open_args = {
                    .tstor_p            = tstor_p,
                    .slot_qhd           = OS_TaskStdInGet(OS_THIS_TASK),
                    .isr_callback_func  = ISR_DrvAudioDeviceCallback
                };
                IF_STATUS(s = OS_AudioDeviceOpen(tstor_p->audio_dev_hd, (void*)&audio_dev_open_args)) { goto error; }
            }
            break;
        default:
            s = S_OK;
            break;
    }
error:
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

/******************************************************************************/
void ISR_DrvAudioDeviceCallback(OS_AudioDeviceCallbackArgs* args_p)
{
//const OS_Signal signal = OS_ISR_SignalCreate(OS_SIG_DRV, args_p->signal_id, 0);
//    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(args_p->slot_qhd, signal, OS_MSG_PRIO_NORMAL));
}

#endif //(OS_AUDIO_ENABLED)