/***************************************************************************//**
* @file    task_shell.c
* @brief   Shell task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include "os_environment.h"
#include "os_driver.h"
#include "os_shell.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#if (1 == USBH_ENABLED)
#if (1 == USBH_HID_ENABLED)
#include "drv_usb.h"
#include "os_task_usbd.h"
#endif //(1 == USBH_HID_ENABLED)
#endif //(1 == USBH_ENABLED)

//-----------------------------------------------------------------------------
#define MDL_NAME            "task_shell"

//-----------------------------------------------------------------------------
//Task arguments
//typedef struct {
//} TaskStorage;

//------------------------------------------------------------------------------
const OS_TaskConfig task_shell_cfg = {
    .name           = "Shell",
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 10,
    .prio_init      = OS_TASK_PRIO_SHELL,
    .prio_power     = OS_TASK_PRIO_PWR_SHELL,
    .storage_size   = 0,//sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN,
    .stdin_len      = 1,
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
extern volatile OS_QueueHd stdin_qhd;
const OS_DriverHd drv_shell = OS_DriverStdInGet();
OS_Message* msg_p;
    OS_TaskPrioritySet(OS_THIS_TASK, OS_TASK_PRIO_LOW);
#if (1 == USBH_ENABLED)
#if (1 == USBH_HID_ENABLED)
const OS_TaskHd usbd_thd = OS_TaskByNameGet(OS_DAEMON_NAME_USBD);
    OS_ASSERT(S_OK == OS_TasksConnect(usbd_thd, OS_THIS_TASK));
#endif //(1 == USBH_HID_ENABLED)
#endif //(1 == USBH_ENABLED)
    //Init stdin_qhd before all other tasks and return to the base priority.
    stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_STDIN:
                        // get char from STDIO driver.
                        OS_ShellClHandler((U8)OS_SignalDataGet(msg_p));
                        break;
                    case OS_SIG_PWR_ACK:
                        break;
                    case OS_SIG_DRV:
                        break;
                    case OS_SIG_TASK_DISCONNECT:
                        break;
                    default:
                        OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                        break;
                }
            } else {
                switch (msg_p->id) {
#if (1 == USBH_ENABLED)
#if (1 == USBH_HID_ENABLED)
                    case OS_MSG_USB_HID_MOUSE:
                        OS_LOG(D_DEBUG, "mouse event");
                        break;
                    case OS_MSG_USB_HID_KEYBOARD: {
                        const OS_UsbHidKeyboardData* keyboard_data_p = (OS_UsbHidKeyboardData*)&(msg_p->data);
                        if ('\0' != keyboard_data_p->key_ascii) {
                            OS_ShellClHandler(keyboard_data_p->key_ascii);
                            if (!OS_StrCmp(OS_EnvVariableGet("echo"), "on")) {
                                OS_TRACE(D_INFO, "%c", keyboard_data_p->key_ascii);
                            }
                        }
                        }
                        break;
#endif //(1 == USBH_HID_ENABLED)
#endif //(1 == USBH_ENABLED)
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
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        default:
            break;
    }
    s = OS_QueueClear(OS_TaskStdInGet(OS_THIS_TASK));
    return s;
}
