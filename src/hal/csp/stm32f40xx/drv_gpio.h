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
    DRV_REQ_GPIO_EXTI_IRQ_ENABLE,
    DRV_REQ_GPIO_EXTI_IRQ_DISABLE,
    DRV_REQ_GPIO_LAST
};

#define FOREACH_GPIO(GPIO)       \
        GPIO(GPIO_DEBUG_1)       \
        GPIO(GPIO_DEBUG_2)       \
        GPIO(GPIO_ASSERT)        \
        GPIO(GPIO_LED_PULSE)     \
        GPIO(GPIO_LED_FS)        \
        GPIO(GPIO_LED_ASSERT)    \
        GPIO(GPIO_LED_USER)      \
        GPIO(GPIO_BUTTON_WAKEUP) \
        GPIO(GPIO_BUTTON_TAMPER) \
        GPIO(GPIO_LAST)          \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_GPIO(GENERATE_ENUM)
} Gpio;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_gpio_v[];

#endif // _DRV_GPIO_H_