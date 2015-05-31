/***************************************************************************//**
* @file    status.h
* @brief   Status codes.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _STATUS_H_
#define _STATUS_H_

#include "typedefs.h"

//------------------------------------------------------------------------------
// debug macros
#if defined(CM4F) || defined(STM32F407xx)
    #include <stdio.h>
    #include <assert.h>

    /// @brief Log levels.
    enum {
        D_NONE,         ///< verbose off
        D_CRITICAL,     ///< critical errors
        D_WARNING,      ///< warnings
        D_INFO,         ///< information
        D_DEBUG         ///< debug
    };
    typedef U8 LogLevel;

//------------------------------------------------------------------------------
    typedef ConstStrP   StatusItem;
    typedef U8          TaskId;                 /// @warning Be sure this type is comform to OS_TaskId!

//------------------------------------------------------------------------------
    ConstStrP StatusStringGet(const Status status, const StatusItem* status_items_p);
    void TracePrint(const LogLevel level, ConstStrP format_cstr, ...);
    void LogPrint(const LogLevel level, TaskId tid, ConstStrP mdl_name_cstr, ConstStrP format_cstr, ...);

    extern const StatusItem status_items_v[];
    #define STATUS_ITEMS_COMMON             &status_items_v[0]
    #define MDL_STATUS_ITEMS STATUS_ITEMS_COMMON
    /// @brief   Writes debug string to the log.
    #define HAL_LOG(level, ...)             LogPrint(level, 0, MDL_NAME, __VA_ARGS__)
    #define HAL_LOG_S(level, status, ...)   HAL_LOG(level, 0, StatusStringGet(status, MDL_STATUS_ITEMS))

    #define HAL_TRACE(level, ...)           TracePrint(level, __VA_ARGS__)
    #define HAL_TRACE_S(level, status)      HAL_TRACE(level, StatusStringGet(status, MDL_STATUS_ITEMS))

    #if (D_DEBUG == HAL_ASSERT_LEVEL)
        void HAL_ASSERT_FAILED(U8* file, U32 line);
        #define HAL_ASSERT(e)               if (!(e)) { HAL_ASSERT_FAILED((U8*)__FILE__, __LINE__); }
        #define HAL_ASSERT_VALUE(e)         HAL_ASSERT(e)
    #else
        #define HAL_ASSERT(e)               if (!(e)) { HAL_CRITICAL_SECTION_ENTER(); HAL_ASSERT_PIN_UP(); while(1) {}; }
        #define HAL_ASSERT_VALUE(e)         ((void)0)
    #endif // HAL_ASSERT_LEVEL_DEFAULT
#else
    #error "status.h: Unknown platform!"
#endif // debug version

/// @brief Status checking macro.
#define IS_STATUS_(s)                       ((Status)S_OK != (Status)(s))

/// @brief Overloaded status checking macro.
/// @details Usage:
///          Status s = S_MDL;
///          IF_STATUS(s) { D_LOG_S(D_WARNING, s); return s; }
///          IF_OK(s) {
///                     ...
///                   }
#define IF_STATUS(s)                        if (IS_STATUS_(s))
#define IF_OK(s)                            if (!IS_STATUS_(s))

/// Status definitions
enum StatusType {
//system
    S_COMMON = ~(S16_MAX),
    S_OK = S_COMMON,            //Ok
//common
    S_INVALID_SIZE,             //Invalid size
    S_INVALID_VALUE,            //Invalid value
    S_INVALID_STATE,            //Invalid state
    S_INVALID_DEVICE,           //Invalid device
    S_INVALID_PTR,              //Invalid pointer
    S_INVALID_CRC,              //Invalid CRC
    S_INVALID_ARG,              //Invalid argument
    S_INVALID_ARGS_COUNT,       //Invalid count of arguments
    S_INVALID_REQ_ID,           //Invalid request id
    S_INVALID_COMMAND,          //Invalid command
    S_UNSUPPORTED,              //Unsupported entity
    S_INVALID_OPERATION,        //Invalid operation
    S_OPERATION_FAILED,         //Operation failed
    S_OVERFLOW,                 //Overflow
    S_OUT_OF_MEMORY,            //Out of memory
    S_OUT_OF_SPACE,             //Out of space
    S_OUT_OF_RANGE,             //Out of range
    S_NOT_EXISTS,               //Not exists
    S_NOT_EMPTY,                //Not empty
    S_IS_EMPTY,                 //Is empty
//states
    S_STOPED,                   //Stopped
    S_ABORTED,                  //Aborted
    S_RESUMED,                  //Resumed
    S_RESETED,                  //Reseted
    S_INITED,                   //Inited
    S_DEINITED,                 //DeInited
    S_OPENED,                   //Opened
    S_CLOSED,                   //Closed
    S_CREATED,                  //Created
    S_DELETED,                  //Deleted
    S_ACCESS_DENIED,            //Access denied
    S_CONNECTED,                //Connected
    S_DISCONNECTED,             //Disconnected
    S_LOCKED,                   //Locked
    S_UNLOCKED,                 //Unlocked
    S_BUSY,                     //Busy
    S_IN_USE,                   //Resource in use
    S_IN_PROGRESS,              //Operation in progress
    S_TIMEOUT,                  //Timeout
//osal
    S_INVALID_SEMAPHORE,        //Invalid semaphore
    S_INVALID_MUTEX,            //Invalid mutex
    S_INVALID_SIGNAL,           //Invalid signal
    S_INVALID_MESSAGE,          //Invalid message
    S_INVALID_QUEUE,            //Invalid queue
    S_INVALID_TASK,             //Invalid task
    S_INVALID_TIMER,            //Invalid timer
    S_INVALID_TRIGGER,          //Invalid trigger
    S_INVALID_DRIVER,           //Invalid driver
//hal
    S_HARDWARE_ERROR,           //Hardware error
    S_DEVICE_ERROR,             //Device error
    S_DRIVER_ERROR,             //Driver error
    S_MEDIA_ERROR,              //Media error
    S_CLASS_ERROR,              //Class error
    S_ITF_ERROR,                //Interface error
    S_IO_ERROR,                 //I/O error
//
    S_UNDEF,                    //Undefined status
//all other cases
    S_MODULE = (~(S16_MAX) / 2),//Software module error
};

#endif // _STATUS_H_
