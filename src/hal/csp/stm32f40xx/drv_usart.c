/**************************************************************************//**
* @file    drv_usart.c
* @brief   USART driver.
* @author  A. Filyanov
* @warning Pay attention for debug macro in this module if stdio is going through UART!
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_usart"

//-----------------------------------------------------------------------------
/// @brief   Init USART.
/// @return  #Status.
Status USART_Init_(void);

extern HAL_DriverItf drv_usart6;

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_usart_v[DRV_ID_USART_LAST];

/*****************************************************************************/
Status USART_Init_(void)
{
    memset(drv_usart_v, 0x0, sizeof(drv_usart_v));
    drv_usart_v[DRV_ID_USART6] = &drv_usart6;
    return S_OK;
}
