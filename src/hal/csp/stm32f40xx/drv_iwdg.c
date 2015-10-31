/**************************************************************************//**
* @file    drv_iwdg.c
* @brief   IWDG driver.
* @author
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_iwdg"

//-----------------------------------------------------------------------------
static IWDG_HandleTypeDef iwdg_handle;

/******************************************************************************/
Status TIMER_IWDG_Init(void)
{
Status s = S_OK;
    __HAL_RCC_LSI_ENABLE();
    while (OS_TRUE != __HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)) {};

    iwdg_handle.Instance        = HAL_TIMER_IWDG_ITF;

    iwdg_handle.Init.Prescaler  = HAL_TIMER_IWDG_PRESCALER;
    iwdg_handle.Init.Reload     = HAL_TIMER_IWDG_TIMEOUT;

    if (HAL_OK != HAL_IWDG_Init(&iwdg_handle)) {
        s = S_HARDWARE_ERROR;
    }
    return s;
}

/******************************************************************************/
Status TIMER_IWDG_Start(void)
{
Status s = S_OK;
    if (HAL_OK != HAL_IWDG_Start(&iwdg_handle)) { s = S_HARDWARE_ERROR; }
    return s;
}

/******************************************************************************/
void TIMER_IWDG_Reset(void)
{
    HAL_IWDG_Refresh(&iwdg_handle);
}
