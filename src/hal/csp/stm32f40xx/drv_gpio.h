/**************************************************************************//**
* @file    drv_gpio.h
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_GPIOA,
    DRV_ID_GPIOB,
    DRV_ID_GPIOC,
    DRV_ID_GPIOD,
    DRV_ID_GPIOE,
    DRV_ID_GPIOF,
    DRV_ID_GPIOG,
    DRV_ID_GPIOH,
    DRV_ID_GPIOI,
    DRV_ID_GPIO_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_gpio_v[];

#endif // _DRV_GPIO_H_