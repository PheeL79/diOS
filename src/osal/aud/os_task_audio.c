/***************************************************************************//**
* @file    task_audio.c
* @brief   Audio daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include "com/cs4344/drv_audio_cs4344.h"
#include "os_debug.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_message.h"
#include "os_audio.h"
#include "os_task_audio.h"

#if (1 == OS_AUDIO_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "task_audio_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd         drv_trimmer_hd;
    OS_AudioDeviceHd    audio_dev_hd;
} TaskArgs;

static TaskArgs task_args;

//-----------------------------------------------------------------------------
const OS_TaskConfig task_audio_cfg = {
    .name       = OS_DAEMON_NAME_AUDIO,
    .func_main  = OS_TaskMain,
    .func_power = OS_TaskPower,
    .args_p     = (void*)&task_args,
    .attrs      = BIT(OS_TASK_ATTR_RECREATE),
    .timeout    = 3,
    .prio_init  = OS_TASK_PRIO_LOW,
    .prio_power = OS_PWR_PRIO_MAX - 10,
    .stack_size = OS_STACK_SIZE_MIN,
    .stdin_len  = OS_STDIN_LEN
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
Status s = S_UNDEF;
//TODO(A. Filyanov) Make a config for an audio(in/out) devices. Like OS FileSystem Media one.
    {
        const OS_DriverConfig drv_cfg = {
            .name       = "TRIMMER",
            .itf_p      = &drv_trimmer,
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args_p->drv_trimmer_hd)) { return s; }
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
        };
        IF_STATUS(s = OS_AudioDeviceCreate(&audio_dev_cfg, &(task_args_p->audio_dev_hd))) { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
OS_Message* msg_p;
Status s = S_UNDEF;

    IF_STATUS_OK(s = OS_DriverInit(task_args_p->drv_trimmer_hd, (void*)&stdin_qhd)) {
        IF_STATUS(s = OS_DriverOpen(task_args_p->drv_trimmer_hd, OS_NULL)) {
        }
    } else {
        s = (S_INIT == s) ? S_OK : s;
    }

	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_DRV: {
                        ConstStrPtr env_var_str_p = "volume";
                        const OS_AudioVolume volume = (OS_AudioVolume)OS_SignalDataGet(msg_p);
                        Str volume_str[4];
                        if (0 > OS_SNPrintF(volume_str, sizeof(volume_str), "%d", volume)) {
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
TaskArgs* task_args_p = (TaskArgs*)args_p;
Status s = S_UNDEF;

    switch (state) {
        case PWR_STARTUP: {
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            }
            break;
        case PWR_OFF:
        case PWR_STOP:
        case PWR_SHUTDOWN: {
            IF_STATUS(s = OS_AudioDeviceClose(task_args_p->audio_dev_hd))  { goto error; }
            IF_STATUS(s = OS_AudioDeviceDeInit(task_args_p->audio_dev_hd)) { goto error; }
            }
            break;
        case PWR_ON: {
            const CS4344_DrvAudioArgsInit drv_args = {
                .freq   = 44100,
                .volume = OS_VolumeGet()
            };
            IF_STATUS(s = OS_AudioDeviceInit(task_args_p->audio_dev_hd, (void*)&drv_args)) { goto error; }
            IF_STATUS(s = OS_AudioDeviceOpen(task_args_p->audio_dev_hd, (void*)&drv_args)) { goto error; }
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

#endif //(1 == OS_FILE_SYSTEM_ENABLED)