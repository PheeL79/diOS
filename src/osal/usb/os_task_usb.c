/***************************************************************************//**
* @file    task_usb.c
* @brief   USB Host/Device daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include "usbh_hid.h"
#include "usbh_msc.h"
//#include "usbd_hid.h"
#include "usbd_msc.h"
#include "os_supervise.h"
#include "os_driver.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_usb.h"
#include "os_task_usb.h"
#include "os_task_fs.h"

#if (1 == USBH_ENABLED) || (1 == USBD_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "task_usb_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd         drv_usbh;
    OS_DriverHd         drv_usbd;
    USBH_HandleTypeDef  usbh_fs_hd;
    USBH_HandleTypeDef  usbh_hs_hd;
    USBD_HandleTypeDef  usbd_fs_hd;
    USBD_HandleTypeDef  usbd_hs_hd;
} TaskArgs;

static TaskArgs task_args;

//-----------------------------------------------------------------------------
const OS_TaskConfig task_usb_cfg = {
    .name       = OS_DAEMON_NAME_USB,
    .func_main  = OS_TaskMain,
    .func_power = OS_TaskPower,
    .args_p     = (void*)&task_args,
    .attrs      = BIT(OS_TASK_ATTR_RECREATE),
    .timeout    = 3,
    .prio_init  = OS_TASK_PRIO_BELOW_NORMAL,
    .prio_power = OS_PWR_PRIO_MAX - 5,
    .stack_size = OS_STACK_SIZE_MIN * 3,
    .stdin_len  = 24
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
#if (1 == USBH_ENABLED)
    {
        const OS_UsbHItfHd usbh_itf_hd = {
            .itf_fs_hd = &(task_args_p->usbh_fs_hd),
            .itf_hs_hd = &(task_args_p->usbh_hs_hd)
        };
        const OS_DriverConfig drv_cfg = {
            .name       = "USBH",
            .itf_p      = drv_usbh_v[DRV_ID_USBH],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args_p->drv_usbh))  { return s; }
        IF_STATUS(s = OS_DriverInit(task_args_p->drv_usbh, (void*)&usbh_itf_hd))        { return s; }
    }
#endif //(1 == USBH_ENABLED)

#if (1 == USBD_ENABLED)
    {
        const OS_UsbDItfHd usbd_itf_hd = {
            .itf_fs_hd = &(task_args_p->usbd_fs_hd),
            .itf_hs_hd = &(task_args_p->usbd_hs_hd)
        };
        const OS_DriverConfig drv_cfg = {
            .name       = "USBD",
            .itf_p      = drv_usbd_v[DRV_ID_USBD],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args_p->drv_usbd))  { return s; }
        IF_STATUS(s = OS_DriverInit(task_args_p->drv_usbd, (void*)&usbd_itf_hd))        { return s; }
    }
#endif //(1 == USBD_ENABLED)
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
const OS_TaskHd fsd_thd = OS_TaskByNameGet(OS_DAEMON_NAME_FS);

    OS_ASSERT(S_OK == OS_TasksConnect(fsd_thd, OS_THIS_TASK));
	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
                const OS_SignalId sig_id = OS_SignalIdGet(msg_p);
                if ((OS_SIG_DRV == sig_id) || (OS_SIG_USBH_EVENT_PORT <= sig_id) && (OS_SIG_USBH_EVENT_URB >= sig_id)) {
#if (1 == USBH_ENABLED) || (1 == USBD_ENABLED)
                    const OS_SignalData sig_data_in = OS_SignalDataGet(msg_p);
                    const OS_UsbItfId usb_itf_id = (OS_UsbItfId)OS_USB_SIG_ITF_GET(sig_data_in);
                    StrPtr usb_itf_str_p;
                    switch (OS_SignalSrcGet(msg_p)) {
#if (1 == USBH_ENABLED)
                        case DRV_ID_USBH: {
                            USBH_HandleTypeDef* usbh_itf_hd_p;
                            if (OS_USB_ID_FS == usb_itf_id) {
                                usb_itf_str_p   = "FS";
                                usbh_itf_hd_p   = &(task_args_p->usbh_fs_hd);
                            } else if (OS_USB_ID_HS == usb_itf_id) {
                                usb_itf_str_p   = "HS";
                                usbh_itf_hd_p   = &(task_args_p->usbh_hs_hd);
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
//                                    OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                                    break;
                            }
                            if (OS_MSG_UNDEF != msg_id) {
                                const OS_UsbEventData usb_ev = {
                                    .itf_hd = (OS_UsbItfHd)usbh_itf_hd_p,
                                    .itf_id = usb_itf_id,
                                    .class  = (OS_UsbClass)USBH_GetActiveClass(usbh_itf_hd_p)
                                };
                                Status s;
                                OS_Message* msg_p = OS_MessageCreate(msg_id, sizeof(usb_ev), OS_BLOCK, &usb_ev);
                                IF_STATUS(s = OS_MessageEmit(msg_p, OS_BLOCK, OS_MSG_PRIO_NORMAL)) {
                                    OS_LOG_S(D_WARNING, s);
                                }
                            }
                            }
                            break;
#endif //(1 == USBH_ENABLED)
#if (1 == USBD_ENABLED)
                        case DRV_ID_USBD: {
//                            USBD_HandleTypeDef* usbd_itf_hd_p;
                            if (OS_USB_ID_FS == usb_itf_id) {
                                usb_itf_str_p   = "FS";
//                                usbd_itf_hd_p   = &(task_args_p->usbd_fs_hd);
                            } else if (OS_USB_ID_HS == usb_itf_id) {
                                usb_itf_str_p   = "HS";
//                                usbd_itf_hd_p   = &(task_args_p->usbd_hs_hd);
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
#endif //(1 == USBD_ENABLED)
                        default:
//                            OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                            break;
                    }
#endif //(1 == USBH_ENABLED) || (1 == USBD_ENABLED)
                }
#if (1 == OS_FILE_SYSTEM_ENABLED)
                else if (OS_SIG_FSD_READY == sig_id) {
#if (1 == USBD_ENABLED)
                    IF_STATUS_OK(OS_DriverOpen(task_args_p->drv_usbd, stdin_qhd)) {}
#endif //(1 == USBD_ENABLED)
                } else if (OS_SIG_TASK_DISCONNECT == sig_id) {
                } else { OS_LOG_S(D_DEBUG, S_UNDEF_SIG); }
#endif //(1 == OS_FILE_SYSTEM_ENABLED)
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
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        case PWR_ON: {
            const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_TaskByNameGet(OS_DAEMON_NAME_USB));
#if (1 == USBH_ENABLED)
            IF_STATUS_OK(s = OS_DriverOpen(task_args_p->drv_usbh, stdin_qhd)) {
#if (1 == USBH_FS_ENABLED)
//                if (USBH_OK != USBH_ReEnumerate(&(task_args_p->usbh_fs_hd))) { s = S_HARDWARE_FAULT; }
#endif // (1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
//                if (USBH_OK != USBH_ReEnumerate(&(task_args_p->usbh_hs_hd))) { s = S_HARDWARE_FAULT; }
#endif // (1 == USBH_HS_ENABLED)
            }
#endif //(1 == USBH_ENABLED)
            }
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
#if (1 == USBH_ENABLED)
            IF_STATUS(s = OS_DriverClose(task_args_p->drv_usbh, OS_NULL)) {}
#endif //(1 == USBH_ENABLED)
#if (1 == USBD_ENABLED)
            IF_STATUS(s = OS_DriverClose(task_args_p->drv_usbd, OS_NULL)) {}
#endif //(1 == USBD_ENABLED)
            break;
        default:
            break;
    }
    return s;
}

#endif //(1 == USBH_ENABLED) || (1 == USBD_ENABLED)