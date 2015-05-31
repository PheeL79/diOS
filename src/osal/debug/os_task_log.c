/***************************************************************************//**
* @file    task_log.c
* @brief   Log\trace\dump task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "os_environment.h"
#include "os_driver.h"
#include "os_shell.h"
#include "os_mailbox.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "task_log"

//-----------------------------------------------------------------------------
//Task arguments
//typedef struct {
//} TaskStorage;

//------------------------------------------------------------------------------
const OS_TaskConfig task_log_cfg = {
    .name           = "Log",
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 10,
    .prio_init      = OS_PRIO_TASK_LOG,
    .prio_power     = OS_PRIO_PWR_TASK_LOG,
    .storage_size   = 0,//sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN + OS_SHELL_HEIGHT,
    .stdin_len      = 16,
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
extern volatile OS_QueueHd stdout_qhd;
ConstStrP shell_prompt_p  = OS_ShellPromptGet();
const U8 shell_prompt_len = OS_StrLen((char const*)shell_prompt_p);
const OS_DriverHd drv_log = OS_DriverStdOutGet();
OS_Message* msg_p;
Bool is_prompted = OS_FALSE;
//    OS_TaskPrioritySet(OS_THIS_TASK, OS_TASK_PRIO_LOW);
    //Init stdout_qhd before all other tasks and return to the base priority.
    stdout_qhd = OS_TaskStdInGet(OS_THIS_TASK);
	for(;;) {
        IF_STATUS(OS_MessageReceive(stdout_qhd, &msg_p, OS_BLOCK)) {
            OS_LOG_S(D_WARNING, S_INVALID_MESSAGE);
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_STDOUT:
                        is_prompted = OS_FALSE;
                        break;
                    case OS_SIG_PWR_ACK:
                        break;
                    default:
                        OS_LOG_S(D_DEBUG, S_INVALID_SIGNAL);
                        break;
                }
            } else {
                switch (msg_p->id) {
                    default:
                        OS_LOG_S(D_DEBUG, S_INVALID_MESSAGE);
                        break;
                }
                OS_MessageDelete(msg_p); // free message allocated memory
            }
        }
        if (OS_TRUE != is_prompted) {
            //If there are no more messages in the input queue - print a shell prompt.
            if (0 == OS_QueueItemsCountGet(stdout_qhd) && (OS_TRUE != is_prompted)) {
                OS_DriverWrite(drv_log, (void*)shell_prompt_p, shell_prompt_len, OS_NULL);
                is_prompted = OS_TRUE;
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
