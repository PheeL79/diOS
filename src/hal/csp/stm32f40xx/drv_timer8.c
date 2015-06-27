/**************************************************************************//**
* @file    drv_timer8.c
* @brief   Timer8 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "drv_timer8"

//-----------------------------------------------------------------------------
/* Definition for TIMx clock resources */
#define TIMx                            TIM8
#define TIMx_CLK_ENABLE()               __TIM8_CLK_ENABLE();

#define TIMx_FORCE_RESET()              __TIM8_FORCE_RESET()
#define TIMx_RELEASE_RESET()            __TIM8_RELEASE_RESET()

#define TIMx_TIMEOUT                    1000

//-----------------------------------------------------------------------------
static Status TIMER8_Init(void* args_p);
static Status TIMER8_DeInit(void* args_p);
static Status TIMER8_LL_Init(void* args_p);
static Status TIMER8_LL_DeInit(void* args_p);
static Status TIMER8_Open(void* args_p);
static Status TIMER8_Close(void* args_p);

//-----------------------------------------------------------------------------
static TIM_HandleTypeDef timer_hd;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_timer8 = {
    .Init   = TIMER8_Init,
    .DeInit = TIMER8_DeInit,
    .Open   = TIMER8_Open,
    .Close  = TIMER8_Close
};

/*****************************************************************************/
Status TIMER8_Init(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init");
    IF_OK(s = TIMER8_LL_Init(OS_NULL)) {
        s = TIMER8_Close(OS_NULL);
    }
    return s;
}

/*****************************************************************************/
Status TIMER8_LL_Init(void* args_p)
{
TIM_MasterConfigTypeDef master_cfg;
Status s = S_OK;
    /* TIMx Periph clock enable */
    TIMx_CLK_ENABLE();
    /* Set TIMx instance */
    timer_hd.Instance = TIMx;
    /* Initialize TIMx peripheral */
    timer_hd.Init.Period            = TIMx_TIMEOUT - 1;
    timer_hd.Init.Prescaler         = ((SystemCoreClock / 2) / TIMx_TIMEOUT) - 1;
    timer_hd.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    timer_hd.Init.CounterMode       = TIM_COUNTERMODE_UP;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&timer_hd));
    /* TIM8 TRGO selection */
    master_cfg.MasterOutputTrigger  = TIM_TRGO_UPDATE;
    master_cfg.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_ASSERT(HAL_OK == HAL_TIMEx_MasterConfigSynchronization(&timer_hd, &master_cfg));
    return s;
}

/*****************************************************************************/
Status TIMER8_LL_DeInit(void* args_p)
{
Status s = S_OK;
    /*##-1- Reset peripherals ##################################################*/
    TIMx_FORCE_RESET();
    TIMx_RELEASE_RESET();
    return s;
}

/*****************************************************************************/
Status TIMER8_DeInit(void* args_p)
{
    return TIMER8_LL_DeInit(OS_NULL);
}

/*****************************************************************************/
Status TIMER8_Open(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_TIM_Base_Start(&timer_hd)) { return s = S_HARDWARE_ERROR; }
    return s;
}

/*****************************************************************************/
Status TIMER8_Close(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_TIM_Base_Stop(&timer_hd)) { return s = S_HARDWARE_ERROR; }
    return s;
}
