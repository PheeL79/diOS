/**************************************************************************//**
* @file    drv_trimmer.c
* @brief   Trimmer driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_trimmer"

//-----------------------------------------------------------------------------
#define TRIMMER_DRV_ADC         DRV_ID_ADC3
#define TRIMMER_DRV_TIMER       DRV_ID_TIMER8

//-----------------------------------------------------------------------------
/// @brief   Init trimmer.
/// @return  #Status.
static Status TRIMMER_Init(void* args_p);
static Status TRIMMER_DeInit(void* args_p);
static Status TRIMMER_Open(void* args_p);
static Status TRIMMER_Close(void* args_p);

//-----------------------------------------------------------------------------
HAL_DriverItf drv_trimmer = {
    .Init   = TRIMMER_Init,
    .DeInit = TRIMMER_DeInit,
    .Open   = TRIMMER_Open,
    .Close  = TRIMMER_Close
};

/*****************************************************************************/
Status TRIMMER_Init(void* args_p)
{
Status s = S_UNDEF;
    IF_STATUS(s = drv_adc_v[TRIMMER_DRV_ADC]->Init(args_p))     { return s; }
    IF_STATUS(s = drv_timer_v[TRIMMER_DRV_TIMER]->Init(OS_NULL)){ return s; }
    return s;
}

/*****************************************************************************/
Status TRIMMER_DeInit(void* args_p)
{
Status s = S_UNDEF;
    IF_STATUS(s = drv_timer_v[TRIMMER_DRV_TIMER]->DeInit(OS_NULL)){ return s; }
    IF_STATUS(s = drv_adc_v[TRIMMER_DRV_ADC]->DeInit(OS_NULL))    { return s; }
    return S_OK;
}

/*****************************************************************************/
Status TRIMMER_Open(void* args_p)
{
Status s = S_UNDEF;
    IF_STATUS(s = drv_adc_v[TRIMMER_DRV_ADC]->Open(OS_NULL))    { return s; }
    IF_STATUS(s = drv_timer_v[TRIMMER_DRV_TIMER]->Open(OS_NULL)){ return s; }
    return S_OK;
}

/*****************************************************************************/
Status TRIMMER_Close(void* args_p)
{
Status s = S_UNDEF;
    IF_STATUS(s = drv_timer_v[TRIMMER_DRV_TIMER]->Close(OS_NULL)){ return s; }
    IF_STATUS(s = drv_adc_v[TRIMMER_DRV_ADC]->Close(OS_NULL))    { return s; }
    return S_OK;
}
