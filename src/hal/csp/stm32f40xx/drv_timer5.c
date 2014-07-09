/**************************************************************************//**
* @file    drv_timer5.c
* @brief   Timer5 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer5"

//-----------------------------------------------------------------------------
#define TIMER_STOPWATCH             TIM5

//-----------------------------------------------------------------------------
Status          TIMER5_Init(void);
static void     TIMER5_Init_(void);
static void     TIMER5_NVIC_Init(void);

//-----------------------------------------------------------------------------
HAL_DriverItf drv_timer5 = {
    .Init   = TIMER5_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status TIMER5_Init(void)
{
    D_LOG(D_INFO, "Init");
    TIMER5_Init_();
    TIMER5_NVIC_Init();
    TIMER5_Reset();
    return S_OK;
}

/*****************************************************************************/
void TIMER5_NVIC_Init(void)
{
NVIC_InitTypeDef NVIC_InitStructure;

    D_LOG(D_INFO, "NVIC Init: ");
    NVIC_InitStructure.NVIC_IRQChannel                  = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    D_TRACE_S(D_INFO, S_OK);
}

// TIMER Stopwatch ------------------------------------------------------------
/*****************************************************************************/
void TIMER5_Init_(void)
{
TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_DeInit(TIMER_STOPWATCH);

    TIM_TimeBaseInitStructure.TIM_Prescaler         = (84) - 1; // (APB1_Clock) - 1
    TIM_TimeBaseInitStructure.TIM_Period            = ~0;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIMER_STOPWATCH, &TIM_TimeBaseInitStructure);
    TIM_ClearITPendingBit(TIMER_STOPWATCH, TIM_IT_Update);
    TIM_ITConfig(TIMER_STOPWATCH, TIM_IT_Update, ENABLE);
}

/*****************************************************************************/
void TIMER5_Reset(void)
{
    TIM_Cmd(TIMER_STOPWATCH, DISABLE);
    TIM_SetCounter(TIMER_STOPWATCH, 0);
}

/*****************************************************************************/
void TIMER5_Start(void)
{
    TIM_Cmd(TIMER_STOPWATCH, ENABLE);
}

/*****************************************************************************/
void TIMER5_Stop(void)
{
    TIM_Cmd(TIMER_STOPWATCH, DISABLE);
}

/*****************************************************************************/
U32 TIMER5_Get(void)
{
    return TIM_GetCounter(TIMER_STOPWATCH);
}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIM5_IRQHandler(void);
void TIM5_IRQHandler(void)
{
    if (TIM_GetITStatus(TIMER_STOPWATCH, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIMER_STOPWATCH, TIM_IT_Update);
        TIMER5_Reset();
        TIMER5_Start();
        //D_ASSERT(FALSE);
    }
}