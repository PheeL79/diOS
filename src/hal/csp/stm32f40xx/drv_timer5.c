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
Status          TIMER5_Init(void);
static void     TIMER5_Init_(void);
//static void     TIMER5_NVIC_Init(void);

//-----------------------------------------------------------------------------
static TIM_HandleTypeDef tim_handle;

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
    HAL_LOG(D_INFO, "Init");
    TIMER5_Init_();
//    TIMER5_NVIC_Init();
    TIMER5_Reset();
    return S_OK;
}

/*****************************************************************************/
//void TIMER5_NVIC_Init(void)
//{
//NVIC_InitTypeDef NVIC_InitStructure;
//
//    HAL_LOG(D_INFO, "NVIC Init: ");
//    NVIC_InitStructure.NVIC_IRQChannel                  = TIM5_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//    D_TRACE_S(D_INFO, S_OK);
//}

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
    if (HAL_OK != HAL_TIM_Base_Init(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }

//TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//
//    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
//    TIM_DeInit(TIMER_STOPWATCH);
//
//    TIM_TimeBaseInitStructure.TIM_Prescaler         = (84) - 1; // (APB1_Clock) - 1
//    TIM_TimeBaseInitStructure.TIM_Period            = ~0;
//    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
//    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
//
//    TIM_TimeBaseInit(TIMER_STOPWATCH, &TIM_TimeBaseInitStructure);
//    TIM_ClearITPendingBit(TIMER_STOPWATCH, TIM_IT_Update);
//    TIM_ITConfig(TIMER_STOPWATCH, TIM_IT_Update, ENABLE);
}

/*****************************************************************************/
void TIMER5_Reset(void)
{
    TIMER5_Stop();
    __HAL_TIM_SetCounter(&tim_handle, 0);
}

/*****************************************************************************/
void TIMER5_Start(void)
{
    if (HAL_OK != HAL_TIM_Base_Start_IT(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
}

/*****************************************************************************/
void TIMER5_Stop(void)
{
    if (HAL_OK != HAL_TIM_Base_Stop_IT(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
}

/*****************************************************************************/
U32 TIMER5_Get(void)
{
    return __HAL_TIM_GetCounter(&tim_handle);
}

///*****************************************************************************/
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIM5_IRQHandler(void);
void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim_handle);
//    if (TIM_GetITStatus(TIMER_STOPWATCH, TIM_IT_Update)) {
//        TIM_ClearITPendingBit(TIMER_STOPWATCH, TIM_IT_Update);
//        TIMER5_Reset();
//        TIMER5_Start();
//        //HAL_ASSERT(FALSE);
//    }
}