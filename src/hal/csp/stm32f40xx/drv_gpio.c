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
static Status   GPIOF_Init(void);
static void     GPIO_DebugPinInit(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_gpio_v[DRV_ID_GPIO_LAST];

//-----------------------------------------------------------------------------
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
    memset(drv_gpio_v, 0x0, sizeof(drv_gpio_v));
    drv_gpio_v[DRV_ID_GPIOF] = &drv_gpiof;
    return S_OK;
}

/*****************************************************************************/
Status GPIOF_Init(void)
{
    D_LOG(D_INFO, "Init: ");
    GPIO_DebugPinInit();
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
void GPIO_DebugPinInit(void)
{
//GPIO_InitTypeDef GPIO_InitStructure;
//
//    GPIO_StructInit(&GPIO_InitStructure);
//    /* Enable the GPIOC clock */
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
//
//    GPIO_InitStructure.GPIO_Pin     = HAL_DEBUG_PIN;
//    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
//    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
//    GPIO_Init(HAL_DEBUG_PIN_PORT, &GPIO_InitStructure);
}
