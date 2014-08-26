/***************************************************************************//**
* @file    task_usbhd.c
* @brief   Shell and log\trace\dump task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "usbh_hid.h"
#include "usbh_msc.h"
#include "os_supervise.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_shell.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_usb.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "task_usbhd"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_DriverHd         drv_usbh;
    OS_QueueHd          stdin_qhd;
    USBH_HandleTypeDef  usbh_hd;
} TaskArgs;

static volatile TaskArgs task_args = {
    .drv_usbh   = OS_NULL,
    .stdin_qhd  = OS_NULL
};

//-----------------------------------------------------------------------------
const OS_TaskConfig task_usbhd_cfg = {
    .name       = "UsbHostD",
    .func_main  = OS_TaskMain,
    .func_power = OS_TaskPower,
    .args_p     = (void*)&task_args,
    .attrs      = OS_TASK_ATTR_RECREATE,
    .timeout    = 1,
    .prio_init  = OS_TASK_PRIO_HIGH,
    .prio_power = OS_PWR_PRIO_MAX,
    .stack_size = OS_STACK_SIZE_MIN,
    .stdin_len  = 24
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
    {
        const OS_DriverConfig drv_cfg = {
            .name       = "USBH",
            .itf_p      = drv_usbh_v[DRV_ID_USBH],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args_p->drv_usbh)) { return s; }
        IF_STATUS(s = OS_DriverInit(task_args_p->drv_usbh, &(task_args_p->usbh_hd)))   { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
OS_Message* msg_p;
	for(;;) {
        IF_STATUS(OS_MessageReceive(task_args_p->stdin_qhd, &msg_p, OS_BLOCK)) {
            //OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SIGNAL_IS(msg_p)) {
                if (DRV_ID_USBH == OS_SIGNAL_SRC_GET(msg_p)) {
                    OS_SignalId sig_id      = OS_SIG_UNDEF;
                    OS_SignalData sig_data  = 0;
                    switch (OS_SIGNAL_ID_GET(msg_p)) {
                        case OS_SIG_DRV:
                            if (DRV_ID_USBH == OS_SIGNAL_SRC_GET(msg_p)) {
                                const U8 id = (U8)(OS_SIGNAL_DATA_GET(msg_p) & U8_MAX);
                                switch (id) {
                                    case HOST_USER_SELECT_CONFIGURATION:
                                        OS_LOG(D_INFO, "Select config");
                                        break;
                                    case HOST_USER_DISCONNECTION:
                                        sig_id = OS_SIG_USB_DISCONNECT;
                                        OS_LOG(D_INFO, "Disconnected");
                                        break;
                                    case HOST_USER_CLASS_ACTIVE:
                                        sig_id = OS_SIG_USB_READY;
                                        OS_LOG(D_INFO, "Ready");
                                        break;
                                    case HOST_USER_CONNECTION:
                                        sig_id = OS_SIG_USB_CONNECT;
                                        OS_LOG(D_INFO, "Connected");
                                        break;
                                    default:
                                        break;
                                }
                            }
                            break;
                        default:
                            //OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                            break;
                    }
                    USBH_Process(&(task_args_p->usbh_hd));
                    if (OS_SIG_UNDEF != sig_id) {
                        switch (USBH_GetActiveClass(&(task_args_p->usbh_hd))) {
                            case USB_MSC_CLASS:
                                sig_data = OS_USB_CLASS_MSC;
                                break;
                            case USB_HID_CLASS:
                                sig_data = OS_USB_CLASS_HID;
                                break;
                            default:
                                break;
                        }
                        const OS_Signal signal = OS_SIGNAL_CREATE(sig_id, sig_data);
                        OS_SIGNAL_EMIT(signal, OS_MSG_PRIO_NORMAL);
                    }
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
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        case PWR_ON:
            task_args_p->stdin_qhd = OS_TaskStdInGet(OS_TaskByNameGet(task_usbhd_cfg.name));
            IF_STATUS_OK(s = OS_DriverOpen(task_args_p->drv_usbh, task_args_p->stdin_qhd)) {
                if (USBH_OK != USBH_ReEnumerate(&(task_args_p->usbh_hd))) { s = S_HARDWARE_FAULT; }
            }
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
            IF_STATUS(s = OS_DriverClose(task_args_p->drv_usbh)) {}
            break;
        default:
            break;
    }
    return s;
}