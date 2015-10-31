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
Status s = S_OK;
    HAL_LOG(L_INFO, "Init");
    trimmer_stdin_qhd = *(OS_QueueHd*)args_p;
    IF_STATUS(s = ADC3_LL_Init(OS_NULL)) { return s; }
    /* ADC Initialization */
    adc3_hd.Instance                    = HAL_ADC_TRIMMER_ITF;
    adc3_hd.Init.ClockPrescaler         = HAL_ADC_TRIMMER_CLOCK_PRESCALER;
    adc3_hd.Init.Resolution             = HAL_ADC_TRIMMER_RESOLUTION;
    adc3_hd.Init.ScanConvMode           = HAL_ADC_TRIMMER_SCAN_CONV_MODE;
    adc3_hd.Init.ContinuousConvMode     = HAL_ADC_TRIMMER_CONT_CONV_MODE;
    adc3_hd.Init.DiscontinuousConvMode  = HAL_ADC_TRIMMER_DISCONT_CONV_MODE;
    adc3_hd.Init.NbrOfDiscConversion    = HAL_ADC_TRIMMER_DISCONT_CONV_NUM;
    adc3_hd.Init.ExternalTrigConvEdge   = HAL_ADC_TRIMMER_EXT_TRIG_CONV_EDGE;
    adc3_hd.Init.ExternalTrigConv       = HAL_ADC_TRIMMER_EXT_TRIG_CONV;
    adc3_hd.Init.DataAlign              = HAL_ADC_TRIMMER_DATA_ALIGN;
    adc3_hd.Init.NbrOfConversion        = HAL_ADC_TRIMMER_CONV_NUM;
    adc3_hd.Init.DMAContinuousRequests  = HAL_ADC_TRIMMER_DMA_CONT_REQ;
    adc3_hd.Init.EOCSelection           = HAL_ADC_TRIMMER_EOC_SELECTION;

    if (HAL_OK != HAL_ADC_Init(&adc3_hd)) { return S_HARDWARE_ERROR; }

    /* Configure ADC regular channel */
    adc_cfg.Channel     = HAL_ADC_TRIMMER_CHANNEL_REG;
    adc_cfg.Rank        = HAL_ADC_TRIMMER_CHANNEL_RANK;
    adc_cfg.SamplingTime= HAL_ADC_TRIMMER_CHANNEL_SAMPLING_TIME;
    adc_cfg.Offset      = HAL_ADC_TRIMMER_CHANNEL_OFFSET;

    if (HAL_OK != HAL_ADC_ConfigChannel(&adc3_hd, &adc_cfg)) { return S_HARDWARE_ERROR; }
    return s;
}

/*****************************************************************************/
Status ADC3_DeInit(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(L_INFO, "DeInit");
    IF_STATUS(s = ADC3_LL_DeInit(OS_NULL)) { return s; }
    return s;
}

/*****************************************************************************/
Status ADC3_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_OK;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO clock ****************************************/
    HAL_ADC_TRIMMER_GPIO_CLK_ENABLE();
    /* ADC Periph clock enable */
    HAL_ADC_TRIMMER_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* ADC Channel GPIO pin configuration */
    GPIO_InitStruct.Pin         = HAL_ADC_TRIMMER_GPIO_PIN;
    GPIO_InitStruct.Mode        = HAL_ADC_TRIMMER_GPIO_MODE;
    GPIO_InitStruct.Pull        = HAL_ADC_TRIMMER_GPIO_PULL;
    GPIO_InitStruct.Speed       = HAL_ADC_TRIMMER_GPIO_SPEED;
    GPIO_InitStruct.Alternate   = HAL_ADC_TRIMMER_GPIO_ALT;
    HAL_GPIO_Init(HAL_ADC_TRIMMER_GPIO_PORT, &GPIO_InitStruct);
    /*##-3- Configure the NVIC #################################################*/
    /* NVIC configuration */
    HAL_NVIC_SetPriority(HAL_ADC_TRIMMER_IRQ, HAL_PRIO_IRQ_ADC_TRIMMER, 0);
    HAL_NVIC_EnableIRQ(HAL_ADC_TRIMMER_IRQ);
    return s;
}

/*****************************************************************************/
Status ADC3_LL_DeInit(void* args_p)
{
Status s = S_OK;
    /*##-1- Reset peripherals ##################################################*/
    HAL_ADC_TRIMMER_FORCE_RESET();
    HAL_ADC_TRIMMER_RELEASE_RESET();
    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-initialize the ADC Channel GPIO pin */
    HAL_GPIO_DeInit(HAL_ADC_TRIMMER_GPIO_PORT, HAL_ADC_TRIMMER_GPIO_PIN);
    return s;
}

/*****************************************************************************/
Status ADC3_Open(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_ADC_Start_IT(&adc3_hd)) { return s = S_HARDWARE_ERROR; }
    return s;
}

/*****************************************************************************/
Status ADC3_Close(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_ADC_Stop_IT(&adc3_hd)) { return s = S_HARDWARE_ERROR; }
    return s;
}
