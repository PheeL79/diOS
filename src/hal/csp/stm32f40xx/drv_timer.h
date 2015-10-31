/**************************************************************************//**
* @file    drv_timer.h
* @brief   Timers driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_TIMER_H_
#define _DRV_TIMER_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_TIMER_IWDG,
    DRV_ID_TIMER1,
    DRV_ID_TIMER2,
    DRV_ID_TIMER3,
    DRV_ID_TIMER4,
    DRV_ID_TIMER5,
    DRV_ID_TIMER6,
    DRV_ID_TIMER7,
    DRV_ID_TIMER8,
    DRV_ID_TIMER9,
    DRV_ID_TIMER10,
    DRV_ID_TIMER11,
    DRV_ID_TIMER12,
    DRV_ID_TIMER13,
    DRV_ID_TIMER14,
    DRV_ID_TIMER_LAST
};

enum {
    DRV_REQ_TIMER_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_TIMER_START,
    DRV_REQ_TIMER_STOP,
    DRV_REQ_TIMER_RESET,
    DRV_REQ_TIMER_COUNTER_GET,
    DRV_REQ_TIMER_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_timer_v[];

//-----------------------------------------------------------------------------
Status      TIMER_IWDG_Init(void);
Status      TIMER_IWDG_Start(void);
void        TIMER_IWDG_Reset(void);

void        TIMER_DWT_Init(void);
void        TIMER_DWT_Start(void);
void        TIMER_DWT_Stop(void);
void        TIMER_DWT_Reset(void);
U32         TIMER_DWT_Get(void);
//TODO(A. Filyanov) Move to the driver's private interface.
void        TIMER10_Reset(void);
void        TIMER10_Start(void);
MutexState  TIMER10_MutexGet(void);

void        TIMER5_Reset(void);
void        TIMER5_Start(void);
void        TIMER5_Stop(void);
U32         TIMER5_Get(void);

//void        TIMER8_Reset(void);
void        TIMER8_Start(void);

#endif // _DRV_TIMER_H_
