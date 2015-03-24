/**************************************************************************//**
* @file    drv_timer10.c
* @brief   Timer10 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer10"

//-----------------------------------------------------------------------------
#define TIMER_TIMESTAMP_IRQ         TIM1_UP_TIM10_IRQn
#define TIMER_TIMESTAMP_IRQ_HANDLER TIM1_UP_TIM10_IRQHandler
#define TIMER_TIMESTAMP_TIMEOUT     (HAL_TIMEOUT_TIMER_TIMESTAMP * KHZ) //ms

//-----------------------------------------------------------------------------
Status          TIMER10_Init(void* args_p);
static void     TIMER10_Init_(void);
void            TIMER10_MutexSet(const MutexState state);

//-----------------------------------------------------------------------------
static TIM_HandleTypeDef timer_hd;
static volatile MutexState mutex_timer_timestamp = UNLOCKED;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_timer10 = {
    .Init   = TIMER10_Init,
};

/*****************************************************************************/
Status TIMER10_Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init");
    TIMER10_Init_();
    TIMER10_Reset();
    return S_OK;
}

/*****************************************************************************/
void TIMER10_Init_(void)
{
    /* TIMx Peripheral clock enable */
    __TIM10_CLK_ENABLE();
    /* Set TIMx instance */
    timer_hd.Instance = TIMER_TIMESTAMP;
    /* Initialize TIMx peripheral */
    timer_hd.Init.Period          = TIMER_TIMESTAMP_TIMEOUT - 1;
    timer_hd.Init.Prescaler       = ((SystemCoreClock / 2) / TIMER_TIMESTAMP_TIMEOUT) - 1;
    timer_hd.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
    timer_hd.Init.CounterMode     = TIM_COUNTERMODE_UP;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&timer_hd));
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(TIMER_TIMESTAMP_IRQ, IRQ_PRIO_TIMER_TIMESTAMP, 0);
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMER_TIMESTAMP_IRQ);
}

/*****************************************************************************/
void TIMER10_Reset(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Stop_IT(&timer_hd));
}

/*****************************************************************************/
void TIMER10_Start(void)
{
    TIMER10_MutexSet(LOCKED);
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Start_IT(&timer_hd));
}

/*****************************************************************************/
MutexState TIMER10_MutexGet(void)
{
    return mutex_timer_timestamp;
}

/*****************************************************************************/
void TIMER10_MutexSet(const MutexState state)
{
    HAL_CRITICAL_SECTION_ENTER(); {
        mutex_timer_timestamp = state;
    } HAL_CRITICAL_SECTION_EXIT();
}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIMER_TIMESTAMP_IRQ_HANDLER(void);
void TIMER_TIMESTAMP_IRQ_HANDLER(void)
{
    HAL_TIM_IRQHandler(&timer_hd);
}
