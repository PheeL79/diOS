/**************************************************************************//**
* @file    drv_timer5.c
* @brief   Timer5 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer5"

//-----------------------------------------------------------------------------
#define TIMER_STOPWATCH             TIM5
#define TIMER_STOPWATCH_IRQ         TIM5_IRQn
#define TIMER_STOPWATCH_IRQ_HANDLER TIM5_IRQHandler

//-----------------------------------------------------------------------------
Status          TIMER5_Init(void* args_p);
static void     TIMER5_Init_(void);

//-----------------------------------------------------------------------------
static TIM_HandleTypeDef tim_handle;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_timer5 = {
    .Init   = TIMER5_Init,
};

/*****************************************************************************/
Status TIMER5_Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init");
    TIMER5_Init_();
    TIMER5_Reset();
    return S_OK;
}

// TIMER Stopwatch ------------------------------------------------------------
/*****************************************************************************/
void TIMER5_Init_(void)
{
    /* TIMx Peripheral clock enable */
    __TIM5_CLK_ENABLE();

    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(TIMER_STOPWATCH_IRQ, OS_PRIORITY_INT_MIN, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMER_STOPWATCH_IRQ);

    /* Set TIMx instance */
    tim_handle.Instance = TIMER_STOPWATCH;

    /* Initialize TIM3 peripheral as follow:
       + Period = 10000 - 1
       + Prescaler = ((SystemCoreClock/2)/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
    */
    tim_handle.Init.Period          = ~0;
    tim_handle.Init.Prescaler       = 0;
    tim_handle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
    tim_handle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&tim_handle));
}

/*****************************************************************************/
void TIMER5_Start(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Start_IT(&tim_handle));
}

/*****************************************************************************/
void TIMER5_Stop(void)
{
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Stop_IT(&tim_handle));
}

/*****************************************************************************/
void TIMER5_Reset(void)
{
    TIMER5_Stop();
    __HAL_TIM_SetCounter(&tim_handle, 0);
}

/*****************************************************************************/
U32 TIMER5_Get(void)
{
    return __HAL_TIM_GetCounter(&tim_handle);
}

///*****************************************************************************/
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    TIMER5_Reset();
//    TIMER5_Start();
//}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIM5_IRQHandler(void);
void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim_handle);
}