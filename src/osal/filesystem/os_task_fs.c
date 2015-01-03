/***************************************************************************//**
* @file    os_task_fs.c
* @brief   File system daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "drv_media_usbh.h"
#include "os_debug.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_message.h"
#include "os_file_system.h"
#include "os_task_fs.h"
#include "os_task_usb.h"

#if (OS_FILE_SYSTEM_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "filesystem_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd             drv_led_fs;
#if defined(OS_MEDIA_VOL_SDRAM)
    OS_FileSystemMediaHd    fs_media_sdram_hd;
#endif //defined(OS_MEDIA_VOL_SDRAM)
#if defined(OS_MEDIA_VOL_SDCARD)
    OS_FileSystemMediaHd    fs_media_sdcard_hd;
#endif //defined(OS_MEDIA_VOL_SDCARD)
#if defined(OS_MEDIA_VOL_USBH_FS)
    OS_FileSystemMediaHd    fs_media_usbh_fs_hd;
#endif //defined(OS_MEDIA_VOL_USBH_FS)
#if defined(OS_MEDIA_VOL_USBH_HS)
    OS_FileSystemMediaHd    fs_media_usbh_hs_hd;
#endif //defined(OS_MEDIA_VOL_USBH_HS)
} TaskStorage;

//-----------------------------------------------------------------------------
const OS_TaskConfig task_fs_cfg = {
    .name           = OS_DAEMON_NAME_FS,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 3,
    .prio_init      = OS_TASK_PRIO_FS,
    .prio_power     = OS_TASK_PRIO_PWR_FS,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN,
    .stdin_len      = OS_STDIN_LEN
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
    {
        //Led FS driver Create/Init
        const OS_DriverConfig drv_cfg = {
            .name       = "LED_FS",
            .itf_p      = drv_led_v[DRV_ID_LED_FS],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, &(tstor_p->drv_led_fs))) { return s; }
        IF_STATUS(s = OS_DriverInit(tstor_p->drv_led_fs, OS_NULL)) { return s; }
    }
#if defined(OS_MEDIA_VOL_SDRAM)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "M_SDRAM",
            .itf_p      = drv_media_v[DRV_ID_MEDIA_SDRAM],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "SDRAM",
            .drv_cfg_p  = &drv_cfg,
            .volume     = OS_MEDIA_VOL_SDRAM
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(tstor_p->fs_media_sdram_hd))) { return s; }
    }
#endif //defined(OS_MEDIA_VOL_SDRAM)
#if defined(OS_MEDIA_VOL_SDCARD)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "M_SDCARD",
            .itf_p      = drv_media_v[DRV_ID_MEDIA_SDCARD],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "SD Card",
            .drv_cfg_p  = &drv_cfg,
            .volume     = OS_MEDIA_VOL_SDCARD
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(tstor_p->fs_media_sdcard_hd))) { return s; }
    }
#endif //defined(OS_MEDIA_VOL_SDCARD)
#if defined(OS_MEDIA_VOL_USBH_FS)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "M_USBHFS",
            .itf_p      = drv_media_v[DRV_ID_MEDIA_USBH_FS],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "USB Flash",
            .drv_cfg_p  = &drv_cfg,
            .volume     = OS_MEDIA_VOL_USBH_FS
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(tstor_p->fs_media_usbh_fs_hd))) { return s; }
    }
#endif //defined(OS_MEDIA_VOL_USBH_FS)
#if defined(OS_MEDIA_VOL_USBH_HS)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "M_USBHHS",
            .itf_p      = drv_media_v[DRV_ID_MEDIA_USBH_HS],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_FileSystemMediaConfig fs_media_cfg = {
            .name       = "USB Flash",
            .drv_cfg_p  = &drv_cfg,
            .volume     = OS_MEDIA_VOL_USBH_HS
        };
        IF_STATUS(s = OS_FileSystemMediaCreate(&fs_media_cfg, &(tstor_p->fs_media_usbh_hs_hd))) { return s; }
    }
#endif //defined(OS_MEDIA_VOL_USBH_HS)

#if (USBH_ENABLED) || (USBD_ENABLED)
    const OS_TaskHd usb_thd = OS_TaskByNameGet(OS_DAEMON_NAME_USB);
    const OS_SignalSrc usb_tid = OS_TaskIdGet(usb_thd);

    OS_ASSERT(S_OK == OS_TasksConnect(OS_THIS_TASK, usb_thd));
    OS_SignalEmit(OS_SignalCreate(OS_SIG_FSD_READY, 0), OS_MSG_PRIO_NORMAL);
#endif //(USBH_ENABLED) || (USBD_ENABLED)
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);

	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
//#if defined(OS_MEDIA_VOL_USBH_FS) || defined(OS_MEDIA_VOL_USBH_HS)
//                if (usb_tid == OS_SignalSrcGet(msg_p)) {
                    const OS_SignalId sig_id = OS_SignalIdGet(msg_p);
//                    const OS_SignalData sig_data = OS_SignalDataGet(msg_p);
//                    const U8 usb_itf_id = OS_USB_SIG_ITF_GET(sig_data);
//                    const U8 usb_msg_id = OS_USB_SIG_MSG_GET(sig_data);
//                    OS_FileSystemMediaHd fs_media_usb_hd;
//                    StrPtr itf_str_p;
//                    StrPtr class_str_p;
//                    StrPtr state_str_p;
//                    // Interface
//                    if (OS_USB_ID_FS == usb_itf_id) {
//                        itf_str_p = "FS";
//#if defined(OS_MEDIA_VOL_USBH_FS)
//                        fs_media_usb_hd = tstor_p->fs_media_usbh_fs_hd;
//#endif //defined(OS_MEDIA_VOL_USBH_FS)
//                    } else if (OS_USB_ID_HS == usb_itf_id) {
//                        itf_str_p = "HS";
//#if defined(OS_MEDIA_VOL_USBH_HS)
//                        fs_media_usb_hd = tstor_p->fs_media_usbh_hs_hd;
//#endif //defined(OS_MEDIA_VOL_USBH_HS)
//                    } else { OS_ASSERT(OS_FALSE); }
//                    // Class
//                    if (OS_USB_CLASS_MSC == usb_msg_id) {
//                        class_str_p = "MSC ";
//                    } else if (OS_USB_CLASS_HID == usb_msg_id) {
//                        class_str_p = "HID ";
//                    } else { class_str_p = ""; }
                    // State
                    switch (sig_id) {
                        case OS_SIG_TASK_DISCONNECT:
                            break;
                        default:
                            OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                            break;
                    }
//                    OS_LOG(D_DEBUG, "%s %s%s", itf_str_p, class_str_p, state_str_p);
//                }
//#endif //defined(OS_MEDIA_VOL_USBH_FS) || defined(OS_MEDIA_VOL_USBH_HS)
            } else {
#if defined(OS_MEDIA_VOL_USBH_FS) || defined(OS_MEDIA_VOL_USBH_HS)
                if ((OS_MSG_USB_CONNECT == msg_p->id) || (OS_MSG_USB_DISCONNECT == msg_p->id)) {
                    OS_FileSystemMediaHd fs_media_usb_hd;
                    const OS_UsbEventData* usb_ev_data_p = (OS_UsbEventData*)&(msg_p->data);
                    const DrvMediaUsbArgsInit drv_args = {
                        .usb_itf_hd = usb_ev_data_p->itf_hd,
                        .drv_led_fs = tstor_p->drv_led_fs
                    };
                    Status s;
                    if (OS_USB_ID_FS == usb_ev_data_p->itf_id) {
#if defined(OS_MEDIA_VOL_USBH_FS)
                        fs_media_usb_hd = tstor_p->fs_media_usbh_fs_hd;
#endif //defined(OS_MEDIA_VOL_USBH_FS)
                    } else if (OS_USB_ID_HS == usb_ev_data_p->itf_id) {
#if defined(OS_MEDIA_VOL_USBH_HS)
                        fs_media_usb_hd = tstor_p->fs_media_usbh_hs_hd;
#endif //defined(OS_MEDIA_VOL_USBH_HS)
                    } else { OS_LOG_S(D_WARNING, S_UNDEF_ITF); }
                    if (OS_USB_CLASS_MSC == usb_ev_data_p->class) {
                        if (OS_MSG_USB_CONNECT == msg_p->id) {
                            IF_OK(s = OS_FileSystemMediaInit(fs_media_usb_hd, (void*)&drv_args)) {
                                if (!OS_StrCmp(OS_EnvVariableGet("media_automount"), "on")) {
                                    IF_OK(s = OS_FileSystemMount(fs_media_usb_hd, OS_NULL)) {
                                    } else { OS_LOG_S(D_WARNING, s); }
                                }
                            } else { OS_LOG_S(D_WARNING, s); }
                        } else if (OS_MSG_USB_DISCONNECT == msg_p->id) {
                            IF_OK(s = OS_FileSystemUnMount(fs_media_usb_hd)) {
                            }
                        } else { OS_LOG_S(D_WARNING, S_UNDEF_CLASS); }
                    }
                }
#endif //defined(OS_MEDIA_VOL_USBH_FS) || defined(OS_MEDIA_VOL_USBH_HS)
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
            IF_STATUS(s = OS_TaskInit(args_p)) {}
            break;
        case PWR_ON:
            IF_STATUS(s = OS_DriverOpen(tstor_p->drv_led_fs, OS_NULL)) { goto error; }
#if defined(OS_MEDIA_VOL_SDRAM)
            IF_STATUS(s = OS_FileSystemMediaInit(tstor_p->fs_media_sdram_hd, &(tstor_p->drv_led_fs))) { goto error; }
            //TODO(A. Filyanov) Emit signal from the driver!
            if (!OS_StrCmp(OS_EnvVariableGet("media_automount"), "on")) {
                IF_STATUS(S_FS_NO_FILESYSTEM == OS_FileSystemMount(tstor_p->fs_media_sdram_hd, OS_NULL)) {
                    IF_OK(OS_FileSystemMake(tstor_p->fs_media_sdram_hd, OS_FS_PART_RULE_FDISK, 0)) {
                    }
                }
            }
#endif //defined(OS_MEDIA_VOL_SDRAM)
#if defined(OS_MEDIA_VOL_SDCARD)
            IF_STATUS(s = OS_FileSystemMediaInit(tstor_p->fs_media_sdcard_hd, &(tstor_p->drv_led_fs))) { goto error; }
            //TODO(A. Filyanov) Emit signal from the driver!
            if (!OS_StrCmp(OS_EnvVariableGet("media_automount"), "on")) {
                IF_OK(OS_FileSystemMount(tstor_p->fs_media_sdcard_hd, OS_NULL)) {
                }
            }
#endif //defined(OS_MEDIA_VOL_SDCARD)
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
#if defined(OS_MEDIA_VOL_SDRAM)
            IF_STATUS(s = OS_FileSystemMediaDeInit(tstor_p->fs_media_sdram_hd)){ goto error; }
#endif //defined(OS_MEDIA_VOL_SDRAM)
#if defined(OS_MEDIA_VOL_SDCARD)
            IF_STATUS(s = OS_FileSystemMediaDeInit(tstor_p->fs_media_sdcard_hd)) { goto error; }
#endif //defined(OS_MEDIA_VOL_SDCARD)
#if defined(OS_MEDIA_VOL_USBH_FS)
            IF_STATUS(s = OS_FileSystemMediaDeInit(tstor_p->fs_media_usbh_fs_hd)) { goto error; }
#endif //defined(OS_MEDIA_VOL_USBH_FS)
#if defined(OS_MEDIA_VOL_USBH_HS)
            IF_STATUS(s = OS_FileSystemMediaDeInit(tstor_p->fs_media_usbh_hs_hd)) { goto error; }
#endif //defined(OS_MEDIA_VOL_USBH_HS)
            if (PWR_SHUTDOWN == state) {
                //Led FS Close/Deinit
                IF_STATUS(s = OS_DriverDeInit(tstor_p->drv_led_fs, OS_NULL)) { goto error; }
            }
            break;
        default:
            break;
    }
error:
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

#endif //(OS_FILE_SYSTEM_ENABLED)