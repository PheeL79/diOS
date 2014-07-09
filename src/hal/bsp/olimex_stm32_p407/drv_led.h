/**************************************************************************//**
* @file    drv_led.h
* @brief   LEDs driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_LED_H_
#define _DRV_LED_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_LED_PULSE,
    DRV_ID_LED_FS,
    DRV_ID_LED_ASSERT,
    DRV_ID_LED_USER,
    DRV_ID_LED_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_led_v[];

#endif // _DRV_LED_H_