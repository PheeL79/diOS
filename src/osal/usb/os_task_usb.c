/***************************************************************************//**
* @file    os_task_usb.c
* @brief   USB Host/Device daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include "drv_usb.h"
#include "usbh_hid.h"
#include "usbh_msc.h"
//#include "usbd_hid.h"
#include "usbd_msc.h"
#include "osal.h"
#include "os_supervise.h"
#include "os_driver.h"
#include "os_mailbox.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_task_usb.h"
#include "os_task_fs.h"
#if (USBD_ENABLED) && (USBD_AUDIO_ENABLED)
#   include "usbd_audio.h"
#   include "os_audio.h"
#endif // (USBD_ENABLED) && (USBD_AUDIO_ENABLED)

#if (USBH_ENABLED) || (USBD_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "usb_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd         drv_usbh;
    OS_DriverHd         drv_usbd;
    USBH_HandleTypeDef  usbh_fs_hd;
    USBH_HandleTypeDef  usbh_hs_hd;
    USBD_HandleTypeDef  usbd_fs_hd;
    USBD_HandleTypeDef  usbd_hs_hd;
#if (USBD_ENABLED) && (USBD_AUDIO_ENABLED)
    OS_AudioDeviceHd    audio_dev_hd;
    OS_AudioDmaMode     audio_dma_mode;
    Bool                audio_buf_idx;
#endif // (USBD_ENABLED) && (USBD_AUDIO_ENABLED)
} TaskStorage;

//-----------------------------------------------------------------------------
#if (USBD_ENABLED) && (USBD_AUDIO_ENABLED)
static void ISR_DrvAudioDeviceCallback(OS_AudioDeviceCallbackArgs* args_p);
#endif // (USBD_ENABLED) && (USBD_AUDIO_ENABLED)

//-----------------------------------------------------------------------------
const OS_TaskConfig task_usb_cfg = {
    .name           = OS_DAEMON_NAME_USB,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 3,
    .prio_init      = OS_TASK_PRIO_USB,
    .prio_power     = OS_TASK_PRIO_PWR_USB,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN * 3,
    .stdin_len      = 64//24
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init");
#if (USBH_ENABLED)
    {
        const OS_UsbHItfHd usbh_itf_hd = {
            .itf_fs_hd = &(tstor_p->usbh_fs_hd),
            .itf_hs_hd = &(tstor_p->usbh_hs_hd)
        };
        const OS_DriverConfig drv_cfg = {
            .name       = "USBH",
            .itf_p      = drv_usbh_v[DRV_ID_USBH],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&tstor_p->drv_usbh))  { return s; }
        IF_STATUS(s = OS_DriverInit(tstor_p->drv_usbh, (void*)&usbh_itf_hd))        { return s; }
    }
#endif // (USBH_ENABLED)

#if (USBD_ENABLED)
    {
        const OS_UsbDItfHd usbd_itf_hd = {
            .itf_fs_hd = &(tstor_p->usbd_fs_hd),
            .itf_hs_hd = &(tstor_p->usbd_hs_hd)
        };
        const OS_DriverConfig drv_cfg = {
            .name       = "USBD",
            .itf_p      = drv_usbd_v[DRV_ID_USBD],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&tstor_p->drv_usbd))  { return s; }
        IF_STATUS(s = OS_DriverInit(tstor_p->drv_usbd, (void*)&usbd_itf_hd))        { return s; }
    }
#endif // (USBD_ENABLED)
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
Status s = S_UNDEF;

	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
                const OS_SignalId sig_id = OS_SignalIdGet(msg_p);
                if ((OS_SIG_DRV == sig_id) || ((OS_SIG_USBH_EVENT_PORT <= sig_id) && (OS_SIG_USBH_EVENT_URB >= sig_id))) {
#if (USBH_ENABLED) || (USBD_ENABLED)
                    const OS_SignalData sig_data_in = OS_SignalDataGet(msg_p);
                    const OS_UsbItfId usb_itf_id = (OS_UsbItfId)OS_USB_SIG_ITF_GET(sig_data_in);
                    StrP usb_itf_str_p;
                    switch (OS_SignalSrcGet(msg_p)) {
#if (USBH_ENABLED)
                        case DRV_ID_USBH:
                            {
                            USBH_HandleTypeDef* usbh_itf_hd_p;
                                if (OS_USB_ID_FS == usb_itf_id) {
                                    usb_itf_str_p   = "FS";
                                    usbh_itf_hd_p   = &(tstor_p->usbh_fs_hd);
                                } else if (OS_USB_ID_HS == usb_itf_id) {
                                    usb_itf_str_p   = "HS";
                                    usbh_itf_hd_p   = &(tstor_p->usbh_hs_hd);
                                } else { OS_ASSERT(OS_FALSE); }
                                USBH_Process(usbh_itf_hd_p);
                                OS_MessageId  msg_id = OS_MSG_UNDEF;
                                switch (OS_USB_SIG_MSG_GET(sig_data_in)) {
                                    case HOST_USER_SELECT_CONFIGURATION:
                                        OS_LOG(D_INFO, "%s Select config", usb_itf_str_p);
                                        break;
                                    case HOST_USER_DISCONNECTION:
                                        OS_LOG(D_INFO, "%s Disconnected", usb_itf_str_p);
                                        msg_id = OS_MSG_USB_DISCONNECT;
                                        break;
                                    case HOST_USER_CLASS_ACTIVE:
                                        OS_LOG(D_INFO, "%s Ready", usb_itf_str_p);
                                        msg_id = OS_MSG_USB_CONNECT;
                                        break;
                                    case HOST_USER_CONNECTION:
                                        OS_LOG(D_INFO, "%s Connected", usb_itf_str_p);
                                        break;
                                    default:
//                                        OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                                        break;
                                }
                                if (OS_MSG_UNDEF != msg_id) {
                                    const OS_UsbEventData usb_ev = {
                                        .itf_hd = (OS_UsbItfHd)usbh_itf_hd_p,
                                        .itf_id = usb_itf_id,
                                        .class  = (OS_UsbClass)USBH_GetActiveClass(usbh_itf_hd_p)
                                    };
                                    Status s;
                                    OS_Message* msg_p = OS_MessageCreate(msg_id, usb_ev, sizeof(usb_ev), OS_BLOCK);
                                    if (OS_NULL != msg_p) {
                                        IF_STATUS(s = OS_MessageEmit(msg_p, OS_BLOCK, OS_MSG_PRIO_NORMAL)) {
                                            OS_LOG_S(D_WARNING, s);
                                        }
                                    } else {
                                        s = S_INVALID_REF;
                                        OS_LOG_S(D_WARNING, s);
                                    }
                                }
                            }
                            break;
#endif // (USBH_ENABLED)
#if (USBD_ENABLED)
                        case DRV_ID_USBD:
                            {
    //                            USBD_HandleTypeDef* usbd_itf_hd_p;
                                if (OS_USB_ID_FS == usb_itf_id) {
                                    usb_itf_str_p   = "FS";
    //                                usbd_itf_hd_p   = &(tstor_p->usbd_fs_hd);
                                } else if (OS_USB_ID_HS == usb_itf_id) {
                                    usb_itf_str_p   = "HS";
    //                                usbd_itf_hd_p   = &(tstor_p->usbd_hs_hd);
                                } else { OS_ASSERT(OS_FALSE); }
                                const U8 usbd_msg_id = OS_USB_SIG_MSG_GET(sig_data_in);
                                switch (usbd_msg_id) {
                                    case OS_SIG_USB_DISCONNECT:
                                        OS_LOG(D_INFO, "%s Disconnected", usb_itf_str_p);
                                        break;
                                    case OS_SIG_USB_CONNECT:
                                        OS_LOG(D_INFO, "%s Connected", usb_itf_str_p);
                                        break;
                                    default:
    //                                    OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                                        break;
                                }
                            }
                            break;
#endif // (USBD_ENABLED)
                        default:
//                            OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                            break;
                    }
#endif // (USBH_ENABLED) || (USBD_ENABLED)
                }
#if (USBD_AUDIO_ENABLED)
                else if (OS_SIG_USB_AUDIO_STOP == sig_id) {
                }
                else if (OS_SIG_USB_AUDIO_PAUSE == sig_id) {
                }
                else if (OS_SIG_USB_AUDIO_RESUME == sig_id) {
                }
                else if (OS_SIG_USB_AUDIO_MUTE_SET == sig_id) {
                }
                else if (OS_SIG_USB_AUDIO_VOLUME_SET == sig_id) {
                }
#endif // (USBD_AUDIO_ENABLED)
#if (OS_FILE_SYSTEM_ENABLED)
                else if (OS_SIG_FSD_READY == sig_id) {
                    const OS_TaskHd fs_thd = OS_TaskByNameGet(OS_DAEMON_NAME_FS);
                    OS_ASSERT(S_OK == OS_TasksConnect(OS_THIS_TASK, fs_thd));
#if (USBD_ENABLED)
                    IF_OK(OS_DriverOpen(tstor_p->drv_usbd, stdin_qhd)) {}
#endif // (USBD_ENABLED)
                } else if (OS_SIG_TASK_DISCONNECT == sig_id) {
                } else { OS_LOG_S(D_DEBUG, S_UNDEF_SIG); }
#endif // (OS_FILE_SYSTEM_ENABLED)
            } else {
                switch (msg_p->id) {
#if (USBD_AUDIO_ENABLED)
                    case OS_MSG_USB_AUDIO_PLAY:
                        if (OS_AUDIO_DMA_MODE_CIRCULAR != tstor_p->audio_dma_mode) {
                            IF_STATUS(s = OS_AudioPlay(tstor_p->audio_dev_hd,
                                                       ((OS_StorageItemLight*)msg_p->data)->data_p,
                                                       ((OS_StorageItemLight*)msg_p->data)->size)) {
                                OS_LOG(D_WARNING, "AudioPlay()");
                            }
                        }
                        break;
                    case OS_MSG_USB_AUDIO_START:
                        IF_STATUS(s = OS_AudioPlay(tstor_p->audio_dev_hd,
                                                   ((OS_StorageItemLight*)msg_p->data)->data_p,
                                                   ((OS_StorageItemLight*)msg_p->data)->size)) {
                            OS_LOG(D_WARNING, "AudioPlay()");
                        }
                        break;
                    case OS_MSG_USB_AUDIO_INIT:
                        {
                        const OS_UsbAudioInitArgs* init_args_p = (OS_UsbAudioInitArgs*)msg_p->data;
                        tstor_p->audio_dev_hd = OS_AudioDeviceDefaultGet(DIR_OUT);
                        if (OS_NULL != tstor_p->audio_dev_hd) {
                            const OS_AudioDeviceIoSetupArgs io_args = {
                                .info       = {
                                    .sample_rate= init_args_p->sample_rate,
                                    .sample_bits= OS_AUDIO_OUT_SAMPLE_BITS_DEFAULT,
                                    .channels   = OS_AUDIO_OUT_CHANNELS_DEFAULT
                                },
                                .dma_mode   = tstor_p->audio_dma_mode = OS_AUDIO_DMA_MODE_CIRCULAR,//OS_AUDIO_DMA_MODE_NORMAL,
                                .volume     = init_args_p->volume
                            };
                            IF_OK(s = OS_AudioDeviceIoSetup(tstor_p->audio_dev_hd, &io_args, DIR_OUT)) {
                                const OS_AudioDeviceArgsOpen audio_dev_open_args = {
                                    .tstor_p            = tstor_p,
                                    .slot_qhd           = OS_TaskStdInGet(OS_THIS_TASK),
                                    .isr_callback_func  = ISR_DrvAudioDeviceCallback
                                };
                                IF_OK(s = OS_AudioDeviceOpen(tstor_p->audio_dev_hd, (void*)&audio_dev_open_args)) {
                                }
                            }
                        }
                        }
                        break;
                    case OS_MSG_USB_AUDIO_DEINIT:
                        {
                        IF_OK(s = OS_AudioStop(tstor_p->audio_dev_hd)) {
                            IF_OK(s = OS_AudioDeviceClose(tstor_p->audio_dev_hd)) {
                            }
                        }
                        }
                        break;
#endif // (USBD_AUDIO_ENABLED)
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
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        case PWR_ON:
            {
            const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_TaskByNameGet(OS_DAEMON_NAME_USB));
#if (USBH_ENABLED)
            IF_OK(s = OS_DriverOpen(tstor_p->drv_usbh, stdin_qhd)) {
#if (USBH_FS_ENABLED)
//                if (USBH_OK != USBH_ReEnumerate(&(tstor_p->usbh_fs_hd))) { s = S_HARDWARE_FAULT; }
#endif // (USBH_FS_ENABLED)
#if (USBH_HS_ENABLED)
//                if (USBH_OK != USBH_ReEnumerate(&(tstor_p->usbh_hs_hd))) { s = S_HARDWARE_FAULT; }
#endif // (USBH_HS_ENABLED)
            }
#endif // (USBH_ENABLED)
            }
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
#if (USBH_ENABLED)
            IF_STATUS(s = OS_DriverClose(tstor_p->drv_usbh, OS_NULL)) {}
#endif // (USBH_ENABLED)
#if (USBD_ENABLED)
            IF_STATUS(s = OS_DriverClose(tstor_p->drv_usbd, OS_NULL)) {}
#endif // (USBD_ENABLED)
            break;
        default:
            break;
    }
    return s;
}

#if (USBD_ENABLED) && (USBD_AUDIO_ENABLED)
/******************************************************************************/
void ISR_DrvAudioDeviceCallback(OS_AudioDeviceCallbackArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->tstor_p;
USBD_HandleTypeDef* usb_itf_hd_p = &(tstor_p->usbd_hs_hd); //TODO(A.Filyanov) Select proper USB interface.

    if (OS_SIG_AUDIO_TX_COMPLETE == args_p->signal_id) {
        if (tstor_p->audio_buf_idx) {
            USBD_AUDIO_Sync(usb_itf_hd_p, AUDIO_OFFSET_FULL);
        } else {
            USBD_AUDIO_Sync(usb_itf_hd_p, AUDIO_OFFSET_HALF);
        }
        tstor_p->audio_buf_idx ^= 1;
    }

//    if (OS_SIG_AUDIO_TX_COMPLETE == args_p->signal_id) {
//        USBD_AUDIO_Sync(usb_itf_hd_p, AUDIO_OFFSET_FULL);
//    } else if (OS_SIG_AUDIO_TX_COMPLETE_HALF == args_p->signal_id) {
//        USBD_AUDIO_Sync(usb_itf_hd_p, AUDIO_OFFSET_HALF);
//    } else if (OS_SIG_AUDIO_ERROR == args_p->signal_id) {
//    } else { OS_ASSERT(OS_FALSE); }
}
#endif // (USBD_ENABLED) && (USBD_AUDIO_ENABLED)

#endif // (USBH_ENABLED) || (USBD_ENABLED)