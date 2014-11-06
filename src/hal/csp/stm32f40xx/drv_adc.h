/**************************************************************************//**
* @file    drv_adc.h
* @brief   ADC driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_ADC_H_
#define _DRV_ADC_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_ADC1,
    DRV_ID_ADC2,
    DRV_ID_ADC3,
    DRV_ID_ADC_LAST
};

enum {
    DRV_REQ_ADC_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_ADC_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_adc_v[];

#endif // _DRV_ADC_H_