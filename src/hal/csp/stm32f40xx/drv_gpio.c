/**************************************************************************//**
* @file    drv_gpio.c
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_debug.h"
#include "os_supervise.h"
#include "os_network.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_gpio"

//-----------------------------------------------------------------------------
/// @brief      GPIO initialize.
/// @return     #Status.
Status          GPIO_Init_(void);
static Status   GPIOA_Init(void* args_p);
static Status   GPIOC_Init(void* args_p);
static void     GPIO_AssertPinInit(void);
static void     GPIO_DebugPinInit(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_gpio_v[DRV_ID_GPIO_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_gpioa = {
    .Init   = GPIOA_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

static HAL_DriverItf drv_gpioc = {
    .Init   = GPIOC_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status GPIO_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_gpio_v, 0x0, sizeof(drv_gpio_v));
    drv_gpio_v[DRV_ID_GPIOA] = &drv_gpioa;
    drv_gpio_v[DRV_ID_GPIOC] = &drv_gpioc;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_gpio_v, drv_gpio_v[0]); ++i) {
        if (OS_NULL != drv_gpio_v[i]) {
            IF_STATUS(s = drv_gpio_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s;
}

/*****************************************************************************/
Status GPIOA_Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init: ");
    GPIO_DebugPinInit();
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
void GPIO_DebugPinInit(void)
{
GPIO_InitTypeDef GPIO_InitStruct;

    HAL_DEBUG_PIN1_CLK_ENABLE();
    HAL_DEBUG_PIN2_CLK_ENABLE();

    GPIO_InitStruct.Pin         = HAL_DEBUG_PIN1 | HAL_DEBUG_PIN2;
    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_HIGH;

    HAL_GPIO_Init(HAL_DEBUG_PIN1_PORT, &GPIO_InitStruct);
}

/*****************************************************************************/
Status GPIOC_Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init: ");
    GPIO_AssertPinInit();
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
void GPIO_AssertPinInit(void)
{
GPIO_InitTypeDef GPIO_InitStruct;

    HAL_ASSERT_PIN_CLK_ENABLE();

    GPIO_InitStruct.Pin         = HAL_ASSERT_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_LOW;

    HAL_GPIO_Init(HAL_ASSERT_PIN_PORT, &GPIO_InitStruct);
}

/*****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
        case GPIO_PIN_0:
            {
            extern HAL_IrqCallbackFunc wakeup_irq_callback_func;
            if (OS_NULL != wakeup_irq_callback_func) {
                wakeup_irq_callback_func();
            }
            }
            break;
        case GPIO_PIN_3:
            {
            extern OS_QueueHd netd_stdin_qhd;
            if (OS_NULL != netd_stdin_qhd) {
                const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_ETH0, OS_SIG_ETH_LINK_STATE_CHANGED, 0);
                OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(netd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
            }
            }
            break;
        default:
            OS_ASSERT(OS_FALSE);
            break;
    }
}