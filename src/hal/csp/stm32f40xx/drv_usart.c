/**************************************************************************//**
* @file    drv_usart.c
* @brief   USART driver.
* @author  A. Filyanov
* @warning Pay attention for debug macro in this module if stdio is going through UART!
******************************************************************************/
#include <string.h>
#include "hal.h"

#include "os_supervise.h"
#include "os_time.h"
#include "os_signal.h"
#include "os_mailbox.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_usart"

//-----------------------------------------------------------------------------
/// @brief   Init USART.
/// @return  #Status.
Status USART_Init_(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_usart_v[DRV_ID_USART_LAST];

/*****************************************************************************/
Status USART_Init_(void)
{
extern HAL_DriverItf drv_usart6;
Status s = S_OK;
    HAL_MemSet(drv_usart_v, 0x0, sizeof(drv_usart_v));
    drv_usart_v[DRV_ID_USART6] = &drv_usart6;
    return s;
}

/******************************************************************************/
/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}

/******************************************************************************/
/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
}

/******************************************************************************/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_ASSERT(OS_FALSE);
}
