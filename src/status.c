/***************************************************************************//**
* @file    status.c
* @brief   Status codes.
* @author  A. Filyanov
*******************************************************************************/
#include <stdarg.h>
#include "printf-stdarg.h"
#include "hal.h"
#include "os_config.h"

INLINE void TraceVaListPrint(ConstStrP format_str_p, va_list args);
INLINE void LogVaListPrint(const LogLevel level, TaskId tid, ConstStrP mdl_name_p, ConstStrP format_str_p, va_list args);

ConstStr log_level_v[D_DEBUG + 1][4] = {
    "",
    "[c]",
    "[w]",
    "[i]",
    "[d]"
};

const StatusItem status_items_v[] = {
//system
    "Ok",
//common
    "Invalid size",
    "Invalid value",
    "Invalid state",
    "Invalid device",
    "Invalid pointer",
    "Invalid CRC",
    "Invalid argument",
    "Invalid count of arguments",
    "Invalid request id",
    "Invalid command",
    "Unsupported entity",
    "Invalid operation",
    "Operation failed",
    "Overflow",
    "Out of memory",
    "Out of space",
    "Out of range",
    "Not exists",
    "Not empty",
    "Is empty",
//states
    "Stopped",
    "Aborted",
    "Resumed",
    "Reseted",
    "Inited",
    "DeInited",
    "Opened",
    "Closed",
    "Created",
    "Deleted",
    "Access denied",
    "Connected",
    "Disconnected",
    "Locked",
    "Unlocked",
    "Busy",
    "Resource in use",
    "Operation in progress",
    "Timeout",
//osal
    "Invalid semaphore",
    "Invalid mutex",
    "Invalid signal",
    "Invalid message",
    "Invalid queue",
    "Invalid task",
    "Invalid timer",
    "Invalid trigger",
    "Invalid driver",
//hal
    "Hardware error",
    "Device error",
    "Driver error",
    "Media error",
    "Class error",
    "Interface error",
    "I/O error",
//
    "Undefined status",
//all other cases
    "Software module error",
};

extern volatile HAL_Env hal_env;

/******************************************************************************/
ConstStrP StatusStringGet(const Status status, const StatusItem* status_items_p)
{
    if ((Status)S_MODULE <= status) {
        if (STATUS_ITEMS_COMMON != status_items_p) {
            if (HAL_NULL != status_items_p) {
                return ((ConstStrP)((StatusItem*)status_items_p)[-(S_MODULE - status)]);
            }
        } else {
            return ((ConstStrP)((StatusItem*)status_items_p)[-(S_COMMON - S_MODULE)]);
        }
    }
    return ((ConstStrP)((StatusItem*)&status_items_v[0])[-(S_COMMON - status)]);
}

/******************************************************************************/
void TracePrint(const LogLevel level, ConstStrP format_str_p, ...)
{
    if (hal_env.log_level >= level) {
        va_list args;
        va_start(args, format_str_p);
        TraceVaListPrint(format_str_p, args);
        va_end(args);
    }
}

/******************************************************************************/
void TraceVaListPrint(ConstStrP format_str_p, va_list args)
{
    vprintf((const char*)format_str_p, args);
}

/******************************************************************************/
void LogPrint(const LogLevel level, TaskId tid, ConstStrP mdl_name_p, ConstStrP format_str_p, ...)
{
    if (hal_env.log_level >= level) {
        va_list args;
        va_start(args, format_str_p);
        LogVaListPrint(level, tid, mdl_name_p, format_str_p, args);
        va_end(args);
    }
}

/******************************************************************************/
void LogVaListPrint(const LogLevel level, TaskId tid, ConstStrP mdl_name_p, ConstStrP format_str_p, va_list args)
{
static U32 log_cycles_last;
UInt elapsed_ms = CYCLES_TO_MS(HAL_CORE_CYCLES - log_cycles_last);
StrP color_str_p;
    log_cycles_last = HAL_CORE_CYCLES;
    // Reset timer value if exec time was more than OS_LOG_TIME_ELAPSED(ms)!
    if (OS_LOG_TIME_ELAPSED < elapsed_ms) {
        elapsed_ms = 0;
    }
    if (D_DEBUG == level) {
        color_str_p = STATUS_COLOR_DEBUG;
    } else if (D_INFO == level) {
        color_str_p = STATUS_COLOR_INFO;
    } else if (D_WARNING == level) {
        color_str_p = STATUS_COLOR_WARNING;
    } else if (D_CRITICAL == level) {
        color_str_p = STATUS_COLOR_CRITICAL;
    } else {
        HAL_ASSERT(HAL_FALSE);
    }
    printf("\n%s%04u %s %03u %-12s :", color_str_p, elapsed_ms, log_level_v[level], tid, mdl_name_p);
    vprintf((const char*)format_str_p, args);
}
