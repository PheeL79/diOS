/***************************************************************************//**
* @file    task_shell.c
* @brief   Shell and log\trace\dump task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "os_environment.h"
#include "os_driver.h"
#include "os_shell.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "task_shell"

//-----------------------------------------------------------------------------
//Task arguments
//typedef struct {
//} TaskArgs;

//static const TaskArgs task_args;

//------------------------------------------------------------------------------
const OS_TaskConfig task_shell_cfg = {
    .name       = "Shell",
    .func_main  = OS_TaskMain,
    .func_power = OS_TaskPower,
    .args_p     = OS_NULL,//(void*)&task_args,
    .attrs      = BIT(OS_TASK_ATTR_RECREATE),
    .timeout    = 10,
    .prio_init  = OS_TASK_PRIO_MAX,
    .prio_power = OS_PWR_PRIO_MAX,
    .stack_size = OS_STACK_SIZE_MIN,
    .stdin_len  = 1,
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
    //Init stdin_qhd before all other tasks and return to the base priority.
    stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            OS_LOG_S(D_WARNING, S_UNDEF_MSG);
        } else {
            if (OS_SIGNAL_IS(msg_p)) {
                switch (OS_SIGNAL_ID_GET(msg_p)) {
                    case OS_SIG_STDIN:
                        // get char from STDIO driver.
                        OS_ShellClHandler((U8)OS_SIGNAL_DATA_GET(msg_p));
                        break;
                    case OS_SIG_PWR_ACK:
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
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        default:
            break;
    }
    s = OS_QueueFlush(OS_TaskStdInGet(OS_THIS_TASK));
    return s;
}
