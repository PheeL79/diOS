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
    HAL_LOG(L_INFO, "Init");
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
    /* TIM Periph clock enable */
    HAL_TIMER_TRIMMER_CLK_ENABLE();
    /* Set TIM instance */
    timer_hd.Instance               = HAL_TIMER_TRIMMER_ITF;
    /* Initialize TIM peripheral */
    timer_hd.Init.Period            = HAL_TIMER_TRIMMER_PERIOD;
    timer_hd.Init.Prescaler         = HAL_TIMER_TRIMMER_PRESCALER;
    timer_hd.Init.ClockDivision     = HAL_TIMER_TRIMMER_CLOCK_DIV;
    timer_hd.Init.CounterMode       = HAL_TIMER_TRIMMER_COUNT_MODE;
    HAL_ASSERT(HAL_OK == HAL_TIM_Base_Init(&timer_hd));
    /* TIM TRGO selection */
    master_cfg.MasterOutputTrigger  = HAL_TIMER_TRIMMER_MASTER_OUT_TRIGGER;
    master_cfg.MasterSlaveMode      = HAL_TIMER_TRIMMER_MASTER_SLAVE_MODE;
    HAL_ASSERT(HAL_OK == HAL_TIMEx_MasterConfigSynchronization(&timer_hd, &master_cfg));
    return s;
}

/*****************************************************************************/
Status TIMER8_LL_DeInit(void* args_p)
{
Status s = S_OK;
    /*##-1- Reset peripherals ##################################################*/
    HAL_TIMER_TRIMMER_FORCE_RESET();
    HAL_TIMER_TRIMMER_RELEASE_RESET();
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
