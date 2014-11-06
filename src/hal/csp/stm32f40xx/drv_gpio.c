/**************************************************************************//**
* @file    drv_gpio.c
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_gpio"

//-----------------------------------------------------------------------------
/// @brief      GPIO initialize.
/// @return     #Status.
Status          GPIO_Init_(void);
static Status   GPIOC_Init(void* args_p);
static Status   GPIOF_Init(void* args_p);
static void     GPIO_AssertPinInit(void);
static void     GPIO_DebugPinInit(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_gpio_v[DRV_ID_GPIO_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_gpioc = {
    .Init   = GPIOC_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

static HAL_DriverItf drv_gpiof = {
    .Init   = GPIOF_Init,
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
    drv_gpio_v[DRV_ID_GPIOC] = &drv_gpioc;
    drv_gpio_v[DRV_ID_GPIOF] = &drv_gpiof;
    for (SIZE i = 0; i < ITEMS_COUNT_GET(drv_gpio_v, drv_gpio_v[0]); ++i) {
        if (OS_NULL != drv_gpio_v[i]) {
            IF_STATUS(s = drv_gpio_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s;
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
Status GPIOF_Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init: ");
    GPIO_DebugPinInit();
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
void GPIO_DebugPinInit(void)
{
//GPIO_InitTypeDef GPIO_InitStruct;

//    HAL_DEBUG_PIN_CLK_ENABLE();

//    GPIO_InitStruct.Pin         = HAL_DEBUG_PIN;
//    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull        = GPIO_NOPULL;
//    GPIO_InitStruct.Speed       = GPIO_SPEED_LOW;
//
//    HAL_GPIO_Init(HAL_DEBUG_PIN_PORT, &GPIO_InitStruct);
}
