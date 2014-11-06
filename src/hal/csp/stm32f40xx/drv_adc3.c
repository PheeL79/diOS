/**************************************************************************//**
* @file    drv_adc3.c
* @brief   ADC3 driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_task.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_adc3"

//-----------------------------------------------------------------------------
/* Definition for ADCx clock resources */
#define ADCx                            ADC3
#define ADCx_CLK_ENABLE()               __ADC3_CLK_ENABLE();
#define ADCx_CHANNEL_GPIO_CLK_ENABLE()  __GPIOC_CLK_ENABLE()

#define ADCx_FORCE_RESET()              __ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN                GPIO_PIN_0
#define ADCx_CHANNEL_GPIO_PORT          GPIOC

/* Definition for ADCx's Channel */
#define ADCx_CHANNEL                    ADC_CHANNEL_10

/* Definition for ADCx's NVIC */
#define ADCx_IRQn                       ADC_IRQn

//-----------------------------------------------------------------------------
static Status ADC3_Init(void* args_p);
static Status ADC3_DeInit(void* args_p);
static Status ADC3_LL_Init(void* args_p);
static Status ADC3_LL_DeInit(void* args_p);
static Status ADC3_Open(void* args_p);
static Status ADC3_Close(void* args_p);

//-----------------------------------------------------------------------------
/*static*/ ADC_HandleTypeDef    adc3_hd;
OS_QueueHd                      trimmer_stdin_qhd;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_adc3 = {
    .Init   = ADC3_Init,
    .DeInit = ADC3_DeInit,
    .Open   = ADC3_Open,
    .Close  = ADC3_Close
};

/*****************************************************************************/
Status ADC3_Init(void* args_p)
{
ADC_ChannelConfTypeDef adc_cfg;
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init");
    trimmer_stdin_qhd = *(OS_QueueHd*)args_p;
    IF_STATUS(s = ADC3_LL_Init(OS_NULL)) { return s; }
    /* ADC Initialization */
    adc3_hd.Instance                    = ADCx;
    adc3_hd.Init.ClockPrescaler         = ADC_CLOCKPRESCALER_PCLK_DIV2;
    adc3_hd.Init.Resolution             = ADC_RESOLUTION8b;
    adc3_hd.Init.ScanConvMode           = ENABLE;
    adc3_hd.Init.ContinuousConvMode     = ENABLE;
    adc3_hd.Init.DiscontinuousConvMode  = DISABLE;
    adc3_hd.Init.NbrOfDiscConversion    = 0;
    adc3_hd.Init.ExternalTrigConvEdge   = ADC_EXTERNALTRIGCONVEDGE_RISING;
    adc3_hd.Init.ExternalTrigConv       = ADC_EXTERNALTRIGCONV_T8_TRGO;
    adc3_hd.Init.DataAlign              = ADC_DATAALIGN_RIGHT;
    adc3_hd.Init.NbrOfConversion        = 1;
    adc3_hd.Init.DMAContinuousRequests  = ENABLE;
    adc3_hd.Init.EOCSelection           = ENABLE;

    if (HAL_OK != HAL_ADC_Init(&adc3_hd)) { return S_HARDWARE_FAULT; }

    /* Configure ADC3 regular channel */
    adc_cfg.Channel     = ADCx_CHANNEL;
    adc_cfg.Rank        = 1;
    adc_cfg.SamplingTime= ADC_SAMPLETIME_15CYCLES;
    adc_cfg.Offset      = 0;

    if (HAL_OK != HAL_ADC_ConfigChannel(&adc3_hd, &adc_cfg)) { return S_HARDWARE_FAULT; }
    return S_OK;
}

/*****************************************************************************/
Status ADC3_DeInit(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "DeInit");
    IF_STATUS(s = ADC3_LL_DeInit(OS_NULL)) { return s; }
    return s;
}

/*****************************************************************************/
Status ADC3_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO clock ****************************************/
    ADCx_CHANNEL_GPIO_CLK_ENABLE();
    /* ADC3 Periph clock enable */
    ADCx_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* ADC3 Channel10 GPIO pin configuration */
    GPIO_InitStruct.Pin = ADCx_CHANNEL_PIN;
    GPIO_InitStruct.Mode= GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull= GPIO_NOPULL;
    HAL_GPIO_Init(ADCx_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    /*##-3- Configure the NVIC #################################################*/
    /* NVIC configuration */
    HAL_NVIC_SetPriority(ADCx_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(ADCx_IRQn);
    return S_OK;
}

/*****************************************************************************/
Status ADC3_LL_DeInit(void* args_p)
{
    /*##-1- Reset peripherals ##################################################*/
    ADCx_FORCE_RESET();
    ADCx_RELEASE_RESET();
    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-initialize the ADC3 Channel10 GPIO pin */
    HAL_GPIO_DeInit(ADCx_CHANNEL_GPIO_PORT, ADCx_CHANNEL_PIN);
    return S_OK;
}

/*****************************************************************************/
Status ADC3_Open(void* args_p)
{
    if (HAL_OK != HAL_ADC_Start_IT(&adc3_hd)) { return S_HARDWARE_FAULT; }
    return S_OK;
}

/*****************************************************************************/
Status ADC3_Close(void* args_p)
{
    if (HAL_OK != HAL_ADC_Stop_IT(&adc3_hd)) { return S_HARDWARE_FAULT; }
    return S_OK;
}
