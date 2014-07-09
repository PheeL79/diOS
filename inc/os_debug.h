/***************************************************************************//**
* @file    os_debug.h
* @brief   OS Debug.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_DEBUG_H_
#define _OS_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "status.h"

/**
* \defgroup OS_Debug OS_Debug
* @{
*/
//------------------------------------------------------------------------------
#define OS_ASSERT(a)        D_ASSERT(a)
#define OS_LOG(level, ...)  OS_Log(level, __VA_ARGS__)

/// @brief Common status items array.
/// @details Usage:
///          Module using common one - do nothing.
///          Module wants to use custom status items array:
///             #undef  MDL_STATUS_ITEMS
///             #define MDL_STATUS_ITEMS        &status_items_app[0] // User status items array
///             //------------------------------------------------------------------------------
///             extern const StatusItem status_items_app[];
///
///             Status s = S_CUSTOM;
///             OS_LOG_S(D_DEBUG, s);
///             OS_LOG_S(D_DEBUG, S_OK);
///          Or you can directly point to the status items array:
///             OS_LOG_S(D_WARNING, S_FS_UNMOUNTED, &status_fs_v[0]);
#define OS_LOG_S(level, status, ...)    OS_Log(level, StatusStringGet(status, MDL_STATUS_ITEMS))
#define OS_TRACE(level, fmt_str_p, ...) OS_Trace(level, fmt_str_p, __VA_ARGS__);

//------------------------------------------------------------------------------
typedef LogLevel OS_LogLevel;           ///< Log level of tracing details.

//------------------------------------------------------------------------------
/// @brief      Init the debug module.
/// @return     #Status.
Status          OS_DebugInit(void);

/// @brief      Deinit the debug module.
/// @return     #Status.
Status          OS_DebugDeInit(void);

/// @brief      Log the message.
/// @param[in]  level          Level of details.
/// @param[in]  format_str_p   Format string pointer.
/// @return     None.
/// @note       Writes log message to the STDOUT with the debug level and the task name.
void            OS_Log(const OS_LogLevel level, ConstStrPtr format_str_p, ...);

/// @brief      Trace the message.
/// @param[in]  level          Level of details.
/// @param[in]  format_str_p   Format string pointer.
/// @return     None.
/// @note       Writes trace message to the STDOUT.
void            OS_Trace(const OS_LogLevel level, ConstStrPtr format_str_p, ...);

/**
* \addtogroup OS_ISR_Debug ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
//void OS_ISR_Log(const OS_LogLevel level, ConstStrPtr format_str_p, ...);
//void OS_ISR_LogS(const OS_LogLevel level, const Status status);
//void OS_ISR_Trace(const OS_LogLevel level, ConstStrPtr format_str_p, ...);

/**@}*/ //OS_ISR_Debug

/**@}*/ //OS_Debug

#ifdef __cplusplus
}
#endif

#endif // _OS_DEBUG_H_
