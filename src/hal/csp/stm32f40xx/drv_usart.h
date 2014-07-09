/**************************************************************************//**
* @file    drv_usart.h
* @brief   USART driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_USART_H_
#define _DRV_USART_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_USART1,
    DRV_ID_USART2,
    DRV_ID_USART3,
    DRV_ID_UART4,
    DRV_ID_UART5,
    DRV_ID_USART6,
    DRV_ID_USART_LAST
};

enum {
    DRV_REQ_USART_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_USART_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_usart_v[];

#endif // _DRV_USART_H_