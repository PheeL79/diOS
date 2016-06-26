/***************************************************************************//**
* @file    hal.h
* @brief   Hardware Abstraction Layer definitions.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _HAL_H_
#define _HAL_H_

#include <stdarg.h>
#include "status.h"

//------------------------------------------------------------------------------
typedef U8 HAL_PowerState;
typedef U8 HAL_PowerPrio;
typedef void (*HAL_ISR_CallbackFunc)(void* args_p);

// Driver standard requests.
enum {
    DRV_REQ_STD_UNDEF = 64,
    DRV_REQ_STD_SYNC,
    DRV_REQ_STD_POWER_SET,
    DRV_REQ_STD_LAST
};

typedef struct {
    Status  (*Init)(void* args_p);
    Status  (*DeInit)(void* args_p);
    Status  (*Open)(void* args_p);
    Status  (*Close)(void* args_p);
    Status  (*Read)(void* data_in_p, Size size, void* args_p);
    Status  (*Write)(void* data_out_p, Size size, void* args_p);
    Status  (*IoCtl)(const U32 request_id, void* args_p);
} HAL_DriverItf;

typedef struct {
    Locale                  locale;
    HAL_PowerState          power;
    const HAL_DriverItf*    stdio_p;
    LogLevel                log_level;
} HAL_Env;

//------------------------------------------------------------------------------
#ifndef HAL_C
extern
#endif
volatile HAL_Env hal_env;

#include "olimex_stm32_p407/hal_olimex_stm32_p407.h"

//------------------------------------------------------------------------------

#endif // _HAL_H_