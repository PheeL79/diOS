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
#define TIMER_TIMESTAMP             TIM10
#define TIMER_TIMESTAMP_IRQ         TIM1_UP_TIM10_IRQn
#define TIMER_TIMESTAMP_IRQ_HANDLER TIM1_UP_TIM10_IRQHandler
#define TIMER_TIMESTAMP_TIMEOUT     (HAL_TIMEOUT_TIMER_TIMESTAMP * KHZ) //ms

//-----------------------------------------------------------------------------
Status          TIMER10_Init(void* args_p);
static void     TIMER10_Init_(void);
//static void     TIMER10_NVIC_Init(void);
static void     TIMER10_MutexSet(const MutexState state);

//-----------------------------------------------------------------------------
static TIM_HandleTypeDef tim_handle;
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
//    TIMER10_NVIC_Init();
    TIMER10_Reset();
    return S_OK;
}

/*****************************************************************************/
//static void TIMER10_NVIC_Init(void)
//{
//NVIC_InitTypeDef NVIC_InitStructure;
//
//    HAL_LOG(D_INFO, "NVIC Init: ");
//    NVIC_InitStructure.NVIC_IRQChannel                  = TIM1_UP_TIM10_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//    D_TRACE_S(D_INFO, S_OK);
//}

// TIMER Timestamp ------------------------------------------------------------
/*****************************************************************************/
void TIMER10_Init_(void)
{
    /* TIMx Peripheral clock enable */
    __TIM10_CLK_ENABLE();

    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(TIMER_TIMESTAMP_IRQ, OS_PRIORITY_INT_MIN, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMER_TIMESTAMP_IRQ);

    /* Set TIMx instance */
    tim_handle.Instance = TIMER_TIMESTAMP;

    /* Initialize TIM3 peripheral as follow:
       + Period = 10000 - 1
       + Prescaler = ((SystemCoreClock/2)/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
    */
    tim_handle.Init.Period          = TIMER_TIMESTAMP_TIMEOUT - 1;
    tim_handle.Init.Prescaler       = (U32)((SystemCoreClock / 2) / TIMER_TIMESTAMP_TIMEOUT) - 1;
    tim_handle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
    tim_handle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    if (HAL_OK != HAL_TIM_Base_Init(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }

//TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//
//    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
//    TIM_DeInit(TIMER_TIMESTAMP);
//
//    TIM_TimeBaseInitStructure.TIM_Prescaler         = (84 * 2) - 1; // (APB2_Clock * APB2_Prescaler) - 1
//    TIM_TimeBaseInitStructure.TIM_Period            = TIMER_TIMESTAMP_TIMEOUT - 1;
//    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Down;
//    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
//
//    TIM_TimeBaseInit(TIMER_TIMESTAMP, &TIM_TimeBaseInitStructure);
//    TIM_ClearITPendingBit(TIMER_TIMESTAMP, TIM_IT_Update);
//    TIM_ITConfig(TIMER_TIMESTAMP, TIM_IT_Update, ENABLE);
}

/*****************************************************************************/
void TIMER10_Reset(void)
{
    if (HAL_OK != HAL_TIM_Base_Stop_IT(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
}

/*****************************************************************************/
void TIMER10_Start(void)
{
    TIMER10_MutexSet(LOCKED);
    if (HAL_OK != HAL_TIM_Base_Start_IT(&tim_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
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

/*****************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (TIMER_TIMESTAMP == htim->Instance) {
        TIMER10_MutexSet(UNLOCKED);
    }
}

// TIMERS IRQ handlers---------------------------------------------------------
/*****************************************************************************/
void TIMER_TIMESTAMP_IRQ_HANDLER(void);
void TIMER_TIMESTAMP_IRQ_HANDLER(void)
{
    HAL_TIM_IRQHandler(&tim_handle);
}
