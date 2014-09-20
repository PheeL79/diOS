/***************************************************************************//**
* @file    task_fsd.c
* @brief   File system daemon task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "os_common.h"
#include "os_debug.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_signal.h"
#include "os_message.h"
#include "os_file_system.h"
#include "os_usb.h"

#if (1 == OS_FILE_SYSTEM_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "task_fsd"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd             drv_led_fs;
#if (1 == SDIO_SD_ENABLED)
    OS_FileSystemMediaHd    fs_sd_hd;
    U8                      volume_sd;
#endif //(1 == SDIO_SD_ENABLED)
#if (1 == USBH_ENABLED)
#if (1 == USBH_FS_ENABLED)
    OS_FileSystemMediaHd    fs_usb_fs_hd;
    U8                      volume_usb_fs;
#endif //(1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
    OS_FileSystemMediaHd    fs_usb_hs_hd;
    U8                      volume_usb_hs;
#endif //(1 == USBH_HS_ENABLED)
#endif //(1 == USBH_ENABLED)
} TaskArgs;

static TaskArgs task_args = {
    .volume_sd      = OS_FILE_SYSTEM_VOL_SD,
    .volume_usb_fs  = OS_FILE_SYSTEM_VOL_USB_FS,
    .volume_usb_hs  = OS_FILE_SYSTEM_VOL_USB_HS
};

//-----------------------------------------------------------------------------
const OS_TaskConfig task_fsd_cfg = {
    .name       = "FileSysD",
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
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
    {
        //Led FS driver Create/Init
        const OS_DriverConfig drv_cfg = {
            .name       = "LED_FS",
            .itf_p      = drv_led_v[DRV_ID_LED_FS],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, &(task_args_p->drv_led_fs))) { return s; }
        IF_STATUS(s = OS_DriverInit(task_args_p->drv_led_fs, OS_NULL)) { return s; }
    }
#if (1 == SDIO_SD_ENABLED)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "SDIO_SD",
            .itf_p      = drv_sdio_v[DRV_ID_SDIO_SD],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "SD Card",
            .drv_cfg_p  = &drv_cfg,
            .volume     = task_args_p->volume_sd
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(task_args_p->fs_sd_hd))) { return s; }
    }
#endif //(1 == SDIO_SD_ENABLED)
#if (1 == USBH_ENABLED)
#if (1 == USBH_FS_ENABLED)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "UHFS_MSC",
            .itf_p      = drv_usbh_v[DRV_ID_USBH_FS_MSC],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "USB Flash",
            .drv_cfg_p  = &drv_cfg,
            .volume     = task_args_p->volume_usb_fs
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(task_args_p->fs_usb_fs_hd))) { return s; }
    }
#endif //(1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "UHHS_MSC",
            .itf_p      = drv_usbh_v[DRV_ID_USBH_HS_MSC],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "USB Flash",
            .drv_cfg_p  = &drv_cfg,
            .volume     = task_args_p->volume_usb_hs
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(task_args_p->fs_usb_hs_hd))) { return s; }
    }
#endif //(1 == USBH_HS_ENABLED)
#endif //(1 == USBH_ENABLED)
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
const OS_TaskHd usbhd_thd = OS_TaskByNameGet("UsbHostD");
const OS_SignalSrc usbhd_tid = OS_TaskIdGet(usbhd_thd);

    OS_ASSERT(S_OK == OS_TasksConnect(usbhd_thd, OS_THIS_TASK));
	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
#if (1 == USBH_ENABLED)
                if (usbhd_tid == OS_SignalSrcGet(msg_p)) {
                    const OS_SignalId sig_id = OS_SignalIdGet(msg_p);
                    const OS_SignalData sig_data = OS_SignalDataGet(msg_p);
                    const U8 usbh_itf_id = OS_USBH_SIG_ITF_GET(sig_data);
                    const U8 usbh_msg_id = OS_USBH_SIG_MSG_GET(sig_data);
                    OS_FileSystemMediaHd fs_usb_hd;
                    StrPtr itf_str_p;
                    StrPtr class_str_p;
                    StrPtr state_str_p;
                    // Interface
                    if (USBH_ID_FS == usbh_itf_id) {
                        itf_str_p = "FS";
                        fs_usb_hd = task_args_p->fs_usb_fs_hd;
                    } else if (USBH_ID_HS == usbh_itf_id) {
                        itf_str_p = "HS";
                        fs_usb_hd = task_args_p->fs_usb_hs_hd;
                    } else { OS_ASSERT(OS_FALSE); }
                    // Class
                    if (OS_USB_CLASS_MSC == usbh_msg_id) {
                        class_str_p = "MSC ";
                    } else if (OS_USB_CLASS_HID == usbh_msg_id) {
                        class_str_p = "HID ";
                    } else { class_str_p = ""; }
                    // State
                    switch (sig_id) {
                        case OS_SIG_USB_CONNECT:
                            state_str_p = "Connected";
                            break;
                        case OS_SIG_USB_READY:
                            state_str_p = "Ready";
                            if (OS_USB_CLASS_MSC == usbh_msg_id) {
                                if (!OS_StrCmp(OS_EnvVariableGet("media_automount"), "on")) {
                                    IF_STATUS_OK(OS_FileSystemMount(fs_usb_hd)) {
                                    }
                                }
                            } else if (OS_USB_CLASS_HID == usbh_msg_id) {
                            }
                            break;
                        case OS_SIG_USB_DISCONNECT:
                            state_str_p = "Disconnected";
                            if (OS_USB_CLASS_MSC == usbh_msg_id) {
                                IF_STATUS_OK(OS_FileSystemUnMount(fs_usb_hd)) {
                                }
                            } else if (OS_USB_CLASS_HID == usbh_msg_id) {
                            }
                            break;
                        case OS_SIG_TASK_DISCONNECT:
                            break;
                        default:
                            OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                            break;
                    }
                    OS_LOG(D_DEBUG, "%s %s%s", itf_str_p, class_str_p, state_str_p);
                }
#endif //(1 == USBH_ENABLED)
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
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {}
            break;
        case PWR_ON:
            IF_STATUS(s = OS_DriverOpen(task_args_p->drv_led_fs, OS_NULL)) { goto error; }
#if (1 == SDIO_SD_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaInit(task_args_p->fs_sd_hd, &(task_args_p->drv_led_fs))) { goto error; }
            //TODO(A. Filyanov) Make an event(signal) from the driver!
            if (!OS_StrCmp(OS_EnvVariableGet("media_automount"), "on")) {
                IF_STATUS_OK(OS_FileSystemMount(task_args_p->fs_sd_hd)) {
                }
            }
#endif //(1 == SDIO_SD_ENABLED)
#if (1 == USBH_ENABLED)
#if (1 == USBH_FS_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaInit(task_args_p->fs_usb_fs_hd, &(task_args_p->drv_led_fs))) { goto error; }
#endif //(1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaInit(task_args_p->fs_usb_hs_hd, &(task_args_p->drv_led_fs))) { goto error; }
#endif //(1 == USBH_HS_ENABLED)
#endif //(1 == USBH_ENABLED)
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
#if (1 == SDIO_SD_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaDeInit(task_args_p->fs_sd_hd)) { goto error; }
#endif //(1 == SDIO_SD_ENABLED)
#if (1 == USBH_ENABLED)
#if (1 == USBH_FS_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaDeInit(task_args_p->fs_usb_fs_hd)) { goto error; }
#endif //(1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
            IF_STATUS(s = OS_FileSystemMediaDeInit(task_args_p->fs_usb_hs_hd)) { goto error; }
#endif //(1 == USBH_HS_ENABLED)
#endif //(1 == USBH_ENABLED)
            //Led FS Close/Deinit
            IF_STATUS(s = OS_DriverDeInit(task_args_p->drv_led_fs, OS_NULL)) { goto error; }
            break;
        default:
            break;
    }
error:
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

#endif //(1 == OS_FILE_SYSTEM_ENABLED)