/***************************************************************************//**
* @file    drv_timer10.c
* @brief   Timer10 driver.
* @author  A. Filyanov
*******************************************************************************/
#include "hal.h"
#include "os_config.h"

//------------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer10"

//------------------------------------------------------------------------------
Status          TIMER10_Init(void* args_p);
static void     TIMER10_Init_(void);
void            TIMER10_MutexSet(const MutexState state);

//------------------------------------------------------------------------------
static TIM_HandleTypeDef timer_hd;
static volatile MutexState mutex_timer_timestamp = UNLOCKED;

//------------------------------------------------------------------------------
HAL_DriverItf drv_timer10 = {
    .Init   = TIMER10_Init,
};

/******************************************************************************/
Status TIMER10_Init(void* args_p)
{
Status s = S_OK;
    HAL_LOG(L_INFO, "Init");
    TIMER10_Init_();
    TIMER10_Reset();
    return s;
}

/******************************************************************************/
void TIMER10_Init_(void)
{
    /* TIMx Peripheral clock enable */
    HAL_TIMER_TIMESTAMP_CLK_ENABLE();
    /* Set TIMx instance */
    timer_hd.Instance           = HAL_TIMER_TIMESTAMP_ITF;
    /* Initialize TIMx peripheral */
    timer_hd.Init.Period        = HAL_TIMER_TIMESTAMP_PERIOD;
    timer_hd.Init.Prescaler     = HAL_TIMER_TIMESTAMP_PRESCALER;
    timer_hd.Init.ClockDivision = HAL_TIMER_TIMESTAMP_CLOCK_DIV;
    timer_hd.Init.CounterMode   = HAL_TIMER_TIMESTAMP_COUNT_MODE;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&timer_hd));
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(HAL_TIMER_TIMESTAMP_IRQ, HAL_PRIO_IRQ_TIMER_TIMESTAMP, 0);
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(HAL_TIMER_TIMESTAMP_IRQ);
}

/******************************************************************************/
void TIMER10_Reset(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Stop_IT(&timer_hd));
}

/******************************************************************************/
void TIMER10_Start(void)
{
    TIMER10_MutexSet(LOCKED);
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Start_IT(&timer_hd));
}

/******************************************************************************/
MutexState TIMER10_MutexGet(void)
{
    return mutex_timer_timestamp;
}

/******************************************************************************/
void TIMER10_MutexSet(const MutexState state)
{
    HAL_CRITICAL_SECTION_ENTER(); {
        mutex_timer_timestamp = state;
    } HAL_CRITICAL_SECTION_EXIT();
}

// IRQ handlers ----------------------------------------------------------------
/******************************************************************************/
void HAL_TIMER_TIMESTAMP_IRQ_HANDLER(void);
void HAL_TIMER_TIMESTAMP_IRQ_HANDLER(void)
{
    HAL_TIM_IRQHandler(&timer_hd);
}
