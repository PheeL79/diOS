/**************************************************************************//**
* @file    drv_led.h
* @brief   LEDs driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_LED_H_
#define _DRV_LED_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_LED,
    DRV_ID_LED_LAST
};

#define FOREACH_LED(LED)    \
        LED(LED_PULSE)      \
        LED(LED_FS)         \
        LED(LED_ASSERT)     \
        LED(LED_USER)       \
        LED(LED_LAST)       \

typedef enum {
    FOREACH_LED(GENERATE_ENUM)
} Led;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_led_v[];

#endif // _DRV_LED_H_