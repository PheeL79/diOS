/***************************************************************************//**
* @file    hal.h
* @brief   HAL.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _HAL_H_
#define _HAL_H_

#include <stdarg.h>
#include "status.h"
#include "olimex_stm32_p407/hal_config.h"

//------------------------------------------------------------------------------
typedef U8 HAL_PowerState;
typedef U8 HAL_PowerPrio;
typedef void (*HAL_IrqCallbackFunc)(void);

// Driver standart requests.
enum {
    DRV_REQ_STD_UNDEF = 64,
    DRV_REQ_STD_SYNC,
    DRV_REQ_STD_MODE_IO_SET,
    DRV_REQ_STD_POWER_SET,
    DRV_REQ_STD_LAST
};

// Driver modes.
enum {
    DRV_MODE_IO_UNDEF,
    DRV_MODE_IO_POLL,
    DRV_MODE_IO_IT,
    DRV_MODE_IO_DMA,
    DRV_MODE_IO_USER,
    DRV_MODE_IO_LAST
};
typedef U8 HAL_DriverModeIo;

typedef struct {
    Status  (*Init)(void);
    Status  (*DeInit)(void);
    Status  (*Open)(void* args_p);
    Status  (*Close)(void);
    Status  (*Read)(U8* data_in_p, U32 size, void* args_p);
    Status  (*Write)(U8* data_out_p, U32 size, void* args_p);
    Status  (*IoCtl)(const U32 request_id, void* args_p);
} HAL_DriverItf;

typedef struct {
    Locale                  locale;
    HAL_PowerState          power;
    const HAL_DriverItf*    stdio_p;
    LogLevel                log_level;
} HAL_Env;

#include "hal/bsp/olimex_stm32_p407/hal_olimex_stm32_p407.h"

//------------------------------------------------------------------------------

#endif // _HAL_H_