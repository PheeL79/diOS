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
    /*##-3- Configure the IWDG peripheral ######################################*/
    /* Set counter reload value to obtain 250ms IWDG TimeOut.
     IWDG counter clock Frequency = LsiFreq / 32
     Counter Reload Value = 250ms / IWDG counter clock period
                          = 0.25s / (32/LsiFreq)
                          = LsiFreq / (32 * 4)
                          = LsiFreq / 128 */
    iwdg_handle.Instance        = IWDG;

    iwdg_handle.Init.Prescaler  = IWDG_PRESCALER_32;
    iwdg_handle.Init.Reload     = TIMER_IWDG_TIMEOUT;

    if (HAL_OK != HAL_IWDG_Init(&iwdg_handle)) {
        s = S_HARDWARE_FAULT;
    }
    return s;
}

/******************************************************************************/
Status TIMER_IWDG_Start(void)
{
Status s = S_OK;
    if (HAL_OK != HAL_IWDG_Start(&iwdg_handle)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
void TIMER_IWDG_Reset(void)
{
    HAL_IWDG_Refresh(&iwdg_handle);
}
