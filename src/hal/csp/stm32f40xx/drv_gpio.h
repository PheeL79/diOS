/**************************************************************************//**
* @file    drv_gpio.h
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_GPIO,
    DRV_ID_GPIO_LAST
};

enum {
    DRV_REQ_GPIO_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_GPIO_TOGGLE,
    DRV_REQ_GPIO_PWM_SET,
    DRV_REQ_GPIO_EXTI_IRQ_ENABLE,
    DRV_REQ_GPIO_EXTI_IRQ_DISABLE,
    DRV_REQ_GPIO_LAST
};

typedef struct {
    Gpio                    gpio;
    HAL_ISR_CallbackFunc    isr_callback_fp;
    void*                   args_p;
} DrvGpioArgsOpen;

typedef struct {
    Gpio                    gpio;
    U32                     pwm_pulse;
} DrvGpioArgsIoCtlPwm;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_gpio_v[];

#endif // _DRV_GPIO_H_