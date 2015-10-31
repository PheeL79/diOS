/***************************************************************************//**
* @file    drv_timer5.c
* @brief   Timer5 driver.
* @author  A. Filyanov
*******************************************************************************/
#include "hal.h"
#include "os_config.h"

//------------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer5"

//------------------------------------------------------------------------------
Status          TIMER5_Init(void* args_p);
static void     TIMER5_Init_(void);

//------------------------------------------------------------------------------
static TIM_HandleTypeDef timer_hd;

//------------------------------------------------------------------------------
HAL_DriverItf drv_timer5 = {
    .Init   = TIMER5_Init,
};

/******************************************************************************/
Status TIMER5_Init(void* args_p)
{
Status s = S_OK;
    HAL_LOG(L_INFO, "Init");
    TIMER5_Init_();
    TIMER5_Reset();
    return s;
}

/******************************************************************************/
void TIMER5_Init_(void)
{
    /* TIMx Peripheral clock enable */
    HAL_TIMER_STOPWATCH_CLK_ENABLE();
    /* Set TIMx instance */
    timer_hd.Instance           = HAL_TIMER_STOPWATCH_ITF;
    /* Initialize TIMx peripheral */
    timer_hd.Init.Period        = HAL_TIMER_STOPWATCH_PERIOD;
    timer_hd.Init.Prescaler     = HAL_TIMER_STOPWATCH_PRESCALER;
    timer_hd.Init.ClockDivision = HAL_TIMER_STOPWATCH_CLOCK_DIV;
    timer_hd.Init.CounterMode   = HAL_TIMER_STOPWATCH_COUNT_MODE;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&timer_hd));
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(HAL_TIMER_STOPWATCH_IRQ, HAL_PRIO_IRQ_TIMER_STOPWATCH, 0);
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(HAL_TIMER_STOPWATCH_IRQ);
}

/******************************************************************************/
void TIMER5_Start(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Start_IT(&timer_hd));
}

/******************************************************************************/
void TIMER5_Stop(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Stop_IT(&timer_hd));
}

/******************************************************************************/
void TIMER5_Reset(void)
{
    TIMER5_Stop();
    __HAL_TIM_SetCounter(&timer_hd, 0);
}

/******************************************************************************/
U32 TIMER5_Get(void)
{
    return __HAL_TIM_GetCounter(&timer_hd);
}

// IRQ handlers ----------------------------------------------------------------
/******************************************************************************/
void HAL_TIMER_STOPWATCH_IRQ_HANDLER(void);
void HAL_TIMER_STOPWATCH_IRQ_HANDLER(void)
{
    HAL_TIM_IRQHandler(&timer_hd);
}