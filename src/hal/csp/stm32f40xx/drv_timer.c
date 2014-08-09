/**************************************************************************//**
* @file    drv_timer.c
* @brief   Timers driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer"

//-----------------------------------------------------------------------------
/// @brief      Init timer.
/// @return     #Status.
Status          TIMER_Init_(void);

extern HAL_DriverItf drv_timer5;
extern HAL_DriverItf drv_timer10;

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_timer_v[DRV_ID_TIMER_LAST];

/*****************************************************************************/
Status TIMER_Init_(void)
{
    HAL_MEMSET(drv_timer_v, 0x0, sizeof(drv_timer_v));
    drv_timer_v[DRV_ID_TIMER5]  = &drv_timer5;
    drv_timer_v[DRV_ID_TIMER10] = &drv_timer10;
    return S_OK;
}

/*****************************************************************************/
void TIMER_DWT_Init(void)
{
   SCB_DEMCR    |= 0x01000000;
   DWT_CYCCNT    = 0; // reset the counter
   DWT_CONTROL  |= 1; // enable the counter
}

/*****************************************************************************/
LNG TIMER_DWT_Get(void)
{
    return (LNG)HAL_CORE_CYCLES;
}
