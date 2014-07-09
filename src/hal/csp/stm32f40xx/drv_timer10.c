/**************************************************************************//**
* @file    drv_timer10.c
* @brief   Timer10 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer10"

//-----------------------------------------------------------------------------
#define TIMER_MILESTONE             TIM10

//ms
#define TIMER_MILESTONE_TIMEOUT     (CFG_TIMER_MILESTONE_TIMEOUT * KHZ)

//-----------------------------------------------------------------------------
Status          TIMER10_Init(void);
static void     TIMER10_Init_(void);
static void     TIMER10_NVIC_Init(void);
static void     TIMER10_MutexSet(const MutexState state);

//-----------------------------------------------------------------------------
static volatile MutexState mutex_timer_milestone = UNLOCKED;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_timer10 = {
    .Init   = TIMER10_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status TIMER10_Init(void)
{
    D_LOG(D_INFO, "Init");
    TIMER10_Init_();
    TIMER10_NVIC_Init();
    TIMER10_Reset();
    return S_OK;
}

/*****************************************************************************/
static void TIMER10_NVIC_Init(void)
{
NVIC_InitTypeDef NVIC_InitStructure;

    D_LOG(D_INFO, "NVIC Init: ");
    NVIC_InitStructure.NVIC_IRQChannel                  = TIM1_UP_TIM10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    D_TRACE_S(D_INFO, S_OK);
}

// TIMER Milestone ------------------------------------------------------------
/*****************************************************************************/
void TIMER10_Init_(void)
{
TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    TIM_DeInit(TIMER_MILESTONE);

    TIM_TimeBaseInitStructure.TIM_Prescaler         = (84 * 2) - 1; // (APB2_Clock * APB2_Prescaler) - 1
    TIM_TimeBaseInitStructure.TIM_Period            = TIMER_MILESTONE_TIMEOUT - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Down;
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIMER_MILESTONE, &TIM_TimeBaseInitStructure);
    TIM_ClearITPendingBit(TIMER_MILESTONE, TIM_IT_Update);
    TIM_ITConfig(TIMER_MILESTONE, TIM_IT_Update, ENABLE);
}

/*****************************************************************************/
void TIMER10_Reset(void)
{
    TIM_Cmd(TIMER_MILESTONE, DISABLE);
    TIM_SetCounter(TIMER_MILESTONE, 0);
}

/*****************************************************************************/
void TIMER10_Start(void)
{
    TIMER10_MutexSet(LOCKED);
    TIM_Cmd(TIMER_MILESTONE, ENABLE);
}

/*****************************************************************************/
MutexState TIMER10_MutexGet(void)
{
    return mutex_timer_milestone;
}

/*****************************************************************************/
void TIMER10_MutexSet(const MutexState state)
{
    mutex_timer_milestone = state;
}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIM1_UP_TIM10_IRQHandler(void);
void TIM1_UP_TIM10_IRQHandler(void)
{
    if (TIM_GetITStatus(TIMER_MILESTONE, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIMER_MILESTONE, TIM_IT_Update);
        TIM_Cmd(TIMER_MILESTONE, DISABLE);
        TIMER10_MutexSet(UNLOCKED);
    }
}
