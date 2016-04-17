/**************************************************************************//**
* @file    hal_olimex_stm32_p407.c
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "hal_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "hal_bsp"

//-----------------------------------------------------------------------------
extern Status   CRC_Init_(void);
extern Status   DMA_Init_(void);
extern Status   RTC_Init_(void);
extern Status   ADC_Init_(void);
extern Status   SPI_Init_(void);
extern Status   I2S_Init_(void);
extern Status   GPIO_Init_(void);
extern Status   USBH__Init(void);
extern Status   USBD__Init(void);
extern Status   TIMER_Init_(void);
extern Status   USART_Init_(void);
extern Status   POWER_Init_(void);
extern Status   ETH_Init_(void);

extern Status   MEM_EXT_Init_(void);
extern Status   BUTTON_Init_(void);
extern Status   MEDIA_Init_(void);
extern Status   AUDIO_Init_(void);

/*****************************************************************************/
Status HAL_BSP_Init(void)
{
Status s = S_UNDEF;
    // Low-level drivers init.
#if (HAL_TIMER_IWDG_ENABLED)
    IF_STATUS(s = TIMER_IWDG_Init()){ return s; }
#endif //(HAL_TIMER_IWDG_ENABLED)
    IF_STATUS(s = POWER_Init_())    { return s; }
    IF_STATUS(s = GPIO_Init_())     { return s; }
    IF_STATUS(s = DMA_Init_())      { return s; }
    IF_STATUS(s = RTC_Init_())      { return s; }
    IF_STATUS(s = CRC_Init_())      { return s; }
#if (HAL_MEM_EXT_ENABLED)
    IF_STATUS(s = MEM_EXT_Init_())  { return s; }
#endif //(HAL_MEM_EXT_ENABLED)
    IF_STATUS(s = ADC_Init_())      { return s; }
    IF_STATUS(s = TIMER_Init_())    { return s; }
    IF_STATUS(s = SPI_Init_())      { return s; }
    IF_STATUS(s = I2S_Init_())      { return s; }
#if (HAL_USBH_ENABLED)
    IF_STATUS(s = USBH__Init())     { return s; }
#endif //(HAL_USBH_ENABLED)
#if (HAL_USBD_ENABLED)
    IF_STATUS(s = USBD__Init())     { return s; }
#endif //(HAL_USBD_ENABLED)
#if (HAL_ETH_ENABLED)
    IF_STATUS(s = ETH_Init_())      { return s; }
#endif //(HAL_ETH_ENABLED)
    // High-level drivers init.
    IF_STATUS(s = BUTTON_Init_())   { return s; }
#if (OS_FILE_SYSTEM_ENABLED)
    IF_STATUS(s = MEDIA_Init_())    { return s; }
#endif //(OS_FILE_SYSTEM_ENABLED)
#if (OS_AUDIO_ENABLED)
    IF_STATUS(s = AUDIO_Init_())    { return s; }
#endif //(OS_AUDIO_ENABLED)
    return s;
}

/******************************************************************************/
void HAL_ExceptionHandler(const HAL_Exception exp)
{
static ConstStr exp_str[][16] = {
    "undefined",
    "nmi",
    "hard fault",
    "mem manage",
    "bus fault",
    "usage fault",
    "svc",
    "debug mon",
    "pend sv",
    "wwdg",
    "pvd"
};
    switch (exp) {
        default:
            break;
    }
    HAL_LOG(L_CRITICAL, "exception: %s", &exp_str[exp][0]);
    HAL_ASSERT(HAL_FALSE);
}