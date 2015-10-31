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

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_timer_v[DRV_ID_TIMER_LAST];

/*****************************************************************************/
Status TIMER_Init_(void)
{
extern HAL_DriverItf drv_timer5;
extern HAL_DriverItf drv_timer8;
extern HAL_DriverItf drv_timer10;
Status s = S_OK;
    HAL_MemSet(drv_timer_v, 0x0, sizeof(drv_timer_v));
    drv_timer_v[DRV_ID_TIMER5]  = &drv_timer5;
    drv_timer_v[DRV_ID_TIMER8]  = &drv_timer8;
    drv_timer_v[DRV_ID_TIMER10] = &drv_timer10;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_timer_v, drv_timer_v[0]); ++i) {
        //Ignore specific driver(s).
        if (DRV_ID_TIMER8 == i) { continue; }
        if (OS_NULL != drv_timer_v[i]) {
            IF_STATUS(s = drv_timer_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s;
}

/*****************************************************************************/
void TIMER_DWT_Init(void)
{
   SCB_DEMCR    |= 0x01000000;
   DWT_CYCCNT    = 0; // reset the counter
   DWT_CONTROL  |= 1; // enable the counter
}

/*****************************************************************************/
void TIMER_DWT_Start(void)
{
    DWT_CONTROL |= (U32)1;   // enable the counter
}

/*****************************************************************************/
void TIMER_DWT_Stop(void)
{
    DWT_CONTROL &= ~((U32)1);// disable the counter
}

/*****************************************************************************/
void TIMER_DWT_Reset(void)
{
    DWT_CYCCNT = (U32)0;    // reset the counter
}

/*****************************************************************************/
U32 TIMER_DWT_Get(void)
{
    return DWT_CYCCNT;
}

/*****************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (HAL_TIMER_TIMESTAMP_ITF == htim->Instance) {
        extern void TIMER10_MutexSet(const MutexState state);
        TIMER10_MutexSet(UNLOCKED);
    }
}