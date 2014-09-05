/***************************************************************************//**
* @file    os_debug.c
* @brief   OS Debug.
* @author  A. Filyanov
*******************************************************************************/
#include <stdarg.h>
#include "printf-stdarg.h"
#include "hal.h"
#include "osal.h"
#include "os_supervise.h"
#include "os_environment.h"
#include "os_signal.h"
#include "os_mutex.h"
#include "os_message.h"
#include "os_debug.h"

//------------------------------------------------------------------------------
extern void TraceVaListPrint(ConstStrPtr format_str_p, va_list args);
extern void LogVaListPrint(const LogLevel level, ConstStrPtr mdl_name_p, ConstStrPtr format_str_p, va_list args);

//------------------------------------------------------------------------------
const TimeMs timeout_def = 100;
volatile static OS_MutexHd print_mut;
volatile OS_QueueHd stdin_qhd;
volatile OS_QueueHd stdout_qhd;

/******************************************************************************/
Status OS_DebugInit(void)
{
    print_mut = OS_MutexCreate();
    if (OS_NULL == print_mut) { return S_INVALID_REF; };
    return S_OK;
}

/******************************************************************************/
Status OS_DebugDeInit(void)
{
    if (OS_NULL == print_mut) { return S_INVALID_REF; };
    OS_MutexDelete(print_mut);
    return S_OK;
}

/******************************************************************************/
void OS_Log(const OS_LogLevel level, ConstStrPtr format_str_p, ...)
{
    IF_STATUS_OK(OS_MutexLock(print_mut, timeout_def)) {
        if (OS_LogLevelGet() >= level) {
            va_list args;
            va_start(args, format_str_p);
            LogVaListPrint(level, OS_TaskNameGet(OS_THIS_TASK), format_str_p, args);
            va_end(args);
            const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_STDOUT, 0);
            OS_SIGNAL_SEND(stdout_qhd, signal, OS_MSG_PRIO_NORMAL);
        }
        OS_MutexUnlock(print_mut);
    }
}

/******************************************************************************/
//void OS_LogS(const OS_LogLevel level, const Status status, ...)
//{
//    va_list args;
//    va_start(args, status);
//    OS_Log(level, StatusStringGet(status, va_arg(args, const StatusItem*)));
//    va_end(args);
//}

/******************************************************************************/
void OS_Trace(const OS_LogLevel level, ConstStrPtr format_str_p, ...)
{
    IF_STATUS_OK(OS_MutexLock(print_mut, timeout_def)) {
        if (OS_LogLevelGet() >= level) {
            va_list args;
            va_start(args, format_str_p);
            TraceVaListPrint(format_str_p, args);
            va_end(args);
            const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_STDOUT, 0);
            OS_SIGNAL_SEND(stdout_qhd, signal, OS_MSG_PRIO_NORMAL);
        }
        OS_MutexUnlock(print_mut);
    }
}

/******************************************************************************/
//void OS_ISR_Log(const OS_LogLevel level, const Status status)
//{
//    OS_Log(level, status);
//}