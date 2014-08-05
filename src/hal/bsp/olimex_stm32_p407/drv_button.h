/**************************************************************************//**
* @file    drv_button.h
* @brief   Board buttons driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_BUTTON_H_
#define _DRV_BUTTON_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_BUTTON_WAKEUP,
    DRV_ID_BUTTON_TAMPER,
    DRV_ID_BUTTON_LAST
};

enum {
    DRV_REQ_BUTTON_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_BUTTON_TAMPER_ENABLE,
    DRV_REQ_BUTTON_TAMPER_DISABLE,
    DRV_REQ_BUTTON_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_button_v[];

#endif // _DRV_BUTTON_H_