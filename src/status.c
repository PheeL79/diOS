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

ConstStr log_level_v[L_LAST][4] = {
    "",
    "[c]",
    "[w]",
    "[i]",
    "[d]",
    "[d]",
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
    "Invalid class",
    "Invalid interface",
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
    TIMER_DWT_Stop();
    if (hal_env.log_level >= level) {
        va_list args;
        va_start(args, format_str_p);
        LogVaListPrint(level, tid, mdl_name_p, format_str_p, args);
        va_end(args);
    }
    TIMER_DWT_Start();
}

/******************************************************************************/
void LogVaListPrint(const LogLevel level, TaskId tid, ConstStrP mdl_name_p, ConstStrP format_str_p, va_list args)
{
static ConstStrP log_level_color_str_v[] = {
    [L_CRITICAL]= STATUS_COLOR_CRITICAL,
    [L_WARNING] = STATUS_COLOR_WARNING,
    [L_INFO]    = STATUS_COLOR_INFO,
    [L_DEBUG_3] = STATUS_COLOR_DEBUG,
    [L_DEBUG_2] = STATUS_COLOR_DEBUG,
    [L_DEBUG_1] = STATUS_COLOR_DEBUG,
};
static U32 log_cycles_last;
UInt elapsed_ms = CYCLES_TO_MS(HAL_CORE_CYCLES - log_cycles_last);
    log_cycles_last = HAL_CORE_CYCLES;
    // Reset timer value if exec time was more than OS_LOG_TIME_ELAPSED(ms)!
    if (OS_LOG_TIME_ELAPSED < elapsed_ms) {
        elapsed_ms = 0;
    }
#if defined(__GNUC__) && defined(USE_SEMIHOSTING)
    printf("%04u %s %03u %-16s:", elapsed_ms, log_level_v[level], tid, mdl_name_p);
    vprintf((const char*)format_str_p, args);
    printf("\r\n");
#else
    printf("\n%s%04u %s %03u %-12s :", log_level_color_str_v[level], elapsed_ms, log_level_v[level], tid, mdl_name_p);
    vprintf((const char*)format_str_p, args);
#endif
}
