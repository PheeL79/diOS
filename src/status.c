/***************************************************************************//**
* @file    status.c
* @brief   Status codes.
* @author  A. Filyanov
*******************************************************************************/
#include <stdarg.h>
#include "printf-stdarg.h"
#include "hal.h"
#include "os_config.h"

void TraceVaListPrint(ConstStrPtr format_str_p, va_list args);
void LogVaListPrint(const LogLevel level, ConstStrPtr mdl_name_p, ConstStrPtr format_str_p, va_list args);

ConstStr log_level_v[D_DEBUG + 1][4] = {
    {""},
    {"[c]"},
    {"[w]"},
    {"[i]"},
    {"[d]"}
};

const StatusItem status_items_v[] = {
//system
    {"Ok"},
    {"Stop"},
    {"Abort"},
    {"Resume"},
    {"Hardware fault"},
    {"Timeout"},
    {"No free memory"},
    {"Open fault"},
    {"Isn't opened"},
    {"Busy"},
    {"Unsupported property"},
    {"Invalid operation"},
    {"Overflow"},
    {"Invalid value"},
    {"Invalid task"},
    {"Invalid state"},
    {"FSM invalid state"},
    {"Invalid number of arguments"},
    {"Invalid reference"},
    {"Undefined parameter"},
    {"Undefined function"},
    {"Undefined queue"},
    {"Undefined event"},
    {"Undefined device"},
    {"Undefined driver"},
    {"Undefined command"},
    {"Undefined message"},
    {"Undefined signal"},
    {"Undefined state"},
    {"Undefined timer"},
    {"Undefined driver request id"},
    {"CRC mismatch"},
    {"Size doesn't match"},
    {"Initialization"},
    {"Isn't initialized"},
    {"App module status"},
    {"Undefined status"}
};

extern volatile HAL_Env hal_env;

/******************************************************************************/
ConstStrPtr StatusStringGet(const Status status, const StatusItem* status_items_p)
{
    if ((S_MODULE <= status) && (S_COMMON > status)) {
        if (STATUS_ITEMS_COMMON != status_items_p) {
            if (OS_NULL != status_items_p) {
                return ((ConstStrPtr)((StatusItem*)status_items_p)[-(S_MODULE - status)]);
            }
        } else {
            return ((ConstStrPtr)((StatusItem*)status_items_p)[-(S_COMMON - S_APP_MODULE)]);
        }
    }
    return ((ConstStrPtr)((StatusItem*)&status_items_v[0])[-(S_COMMON - status)]);
}

/******************************************************************************/
void TracePrint(const LogLevel level, ConstStrPtr format_str_p, ...)
{
    if (hal_env.log_level >= level) {
        va_list args;
        va_start(args, format_str_p);
        TraceVaListPrint(format_str_p, args);
        va_end(args);
    }
}

/******************************************************************************/
void TraceVaListPrint(ConstStrPtr format_str_p, va_list args)
{
    vprintf((const char*)format_str_p, args);
}

/******************************************************************************/
void LogPrint(const LogLevel level, ConstStrPtr mdl_name_p, ConstStrPtr format_str_p, ...)
{
    if (hal_env.log_level >= level) {
        va_list args;
        va_start(args, format_str_p);
        LogVaListPrint(level, mdl_name_p, format_str_p, args);
        va_end(args);
    }
}

/******************************************************************************/
void LogVaListPrint(const LogLevel level, ConstStrPtr mdl_name_p, ConstStrPtr format_str_p, va_list args)
{
static U32 log_cycles_last;
    U32 op_time_ms  = CYCLES_TO_MS(HAL_CORE_CYCLES - log_cycles_last);
    log_cycles_last = HAL_CORE_CYCLES;
    // Reset timer value if exec time was more than OS_LOG_TIMER_STEP(ms)!
    if (OS_LOG_TIMER_STEP < op_time_ms) {
        op_time_ms = 0;
    }
    printf("\n%04d %s %-12s :", op_time_ms, log_level_v[level], mdl_name_p);
    vprintf((const char*)format_str_p, args);
}
