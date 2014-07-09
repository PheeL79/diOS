/***************************************************************************//**
* @file    status.h
* @brief   Status codes.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _STATUS_H_
#define _STATUS_H_

#include "typedefs.h"

//------------------------------------------------------------------------------
#define STATUS_MAX      256       //status count(max 65535)

//------------------------------------------------------------------------------
// debug macros
#if defined(QT_VERSION)
#elif defined(CM4F) || defined(STM32F40XX)
    #include <stdio.h>
    #include <assert.h>

    /// @brief Log levels.
    typedef enum {
        D_NONE,         ///< verbose off
        D_CRITICAL,     ///< critical errors
        D_WARNING,      ///< warnings
        D_INFO,         ///< information
        D_DEBUG         ///< debug
    } LogLevel;

//------------------------------------------------------------------------------
//    typedef struct {
//        Status          status;                 ///< Status.
//        ConstStrPtr     string_p;               ///< Status description string.
//    } StatusItem;

    typedef ConstStrPtr StatusItem;

//------------------------------------------------------------------------------
    ConstStrPtr StatusStringGet(const Status status, const StatusItem* status_items_p);
    void TracePrint(const LogLevel level, ConstStrPtr format_str_p, ...);
    void LogPrint(const LogLevel level, ConstStrPtr mdl_name_p, ConstStrPtr format_str_p, ...);

    extern const StatusItem status_items_v[];
    #define STATUS_ITEMS_COMMON             &status_items_v[0]
    #define MDL_STATUS_ITEMS STATUS_ITEMS_COMMON
    /// @brief   Writes debug string to the log.
    #define D_LOG(level, ...)               LogPrint(level, MDL_NAME, __VA_ARGS__)
    #define D_LOG_S(level, status, ...)     D_LOG(level, StatusStringGet(status, MDL_STATUS_ITEMS))

    #define D_TRACE(level, ...)             TracePrint(level, __VA_ARGS__)
    #define D_TRACE_S(level, status)        D_TRACE(level, StatusStringGet(status, MDL_STATUS_ITEMS))
//#undef MDL_STATUS_ITEMS
//    #ifndef NDEBUG
//        #define D_ASSERT(c)                 assert(c)
//    #else
        #define D_ASSERT(c)                 if (!(c)) { CRITICAL_ENTER(); HAL_ASSERT_PIN_UP; while(1) {}; }
//    #endif // D_ASSERT
#else
    #error "status.h: Undefined compiler"
#endif // debug version

/// @brief Status checking macro.
#define IS_STATUS_(s)                       ((Status)(s) != (Status)S_OK)

/// @brief Overloaded status checking macro.
/// @details Usage:
///          Status s = S_MDL;
///          IF_STATUS(s) { D_LOG_S(D_WARNING, s); return s; }
#define IF_STATUS(s)                        if (IS_STATUS_(s))
#define IF_STATUS_OK(s)                     if (!IS_STATUS_(s))

// Common status
#define S_COMMON                            (-64)
// Application module status
#define S_MODULE                            (-128)

/// Status definitions
enum StatusType {
//system
    S_OK = S_COMMON,
    S_STOP,
    S_ABORT,
    S_RESUME,
    S_HARDWARE_FAULT,
    S_TIMEOUT,
    S_NO_MEMORY,
    S_OPEN,
    S_ISNT_OPENED,
    S_BUSY,
    S_UNSUPPORTED,
    S_INVALID_OPERATION,
    S_OVERFLOW,
    S_INVALID_VALUE,
    S_INVALID_TASK,
    S_INVALID_STATE,
    S_INVALID_STATE_FSM,
    S_INVALID_ARGS_NUMBER,
    S_INVALID_REF,
    S_UNDEF_PARAMETER,
    S_UNDEF_FUNCTION,
    S_UNDEF_QUEUE,
    S_UNDEF_EVENT,
    S_UNDEF_DEVICE,
    S_UNDEF_DRV,
    S_UNDEF_CMD,
    S_UNDEF_MSG,
    S_UNDEF_SIG,
    S_UNDEF_STATE,
    S_UNDEF_TIMER,
    S_UNDEF_REQ_ID,
    S_CRC_MISMATCH,
    S_SIZE_MISMATCH,
    S_INIT,
    S_ISNT_INITED,
    S_APP_MODULE,
//all other cases
    S_LAST
};

#endif // _STATUS_H_
