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

//TODO(A. Filyanov) Substitute type for "slot_qhd" with "OS_DriverHd", "signal_data" with "OS_SignalData"
//                  and "signal_id" with "OS_SignalId" and when header files dependency will solved.
typedef struct {
    Gpio            gpio;
    U8              signal_id;
    U16             signal_data;
    void*           slot_qhd;
} DrvGpioArgsOpen;

typedef struct {
    U32             pwm_pulse;
    Gpio            gpio;
} DrvGpioArgsIoCtlPwm;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_gpio_v[];

#endif // _DRV_GPIO_H_