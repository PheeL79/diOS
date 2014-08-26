/**************************************************************************//**
* @file    hal_olimex_stm32_p407.c
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#include <yfuns.h>
#include <string.h>
#include "hal.h"
#include "hal_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "hal_bsp"

//-----------------------------------------------------------------------------
extern Status   DMA_Init_(void);
extern Status   RTC_Init_(void);
extern Status   NVIC_Init_(void);
extern Status   GPIO_Init_(void);
extern Status   SDIO__Init(void);
extern Status   USBH__Init(void);
extern Status   TIMER_Init_(void);
extern Status   USART_Init_(void);
extern Status   POWER_Init_(void);

extern Status   MEM_EXT_Init_(void);
extern Status   BUTTON_Init_(void);
extern Status   LED_Init_(void);

extern volatile HAL_Env hal_env;

/*****************************************************************************/
Status HAL_BSP_Init(void)
{
Status s = S_OK;
    // Low-level drivers init.
#if (1 == TIMER_IWDG_ENABLED)
    IF_STATUS(s = TIMER_IWDG_Init()) { return s; }
#endif // TIMER_IWDG_ENABLED
    IF_STATUS(s = RTC_Init_()) { return s; }
    //IF_STATUS(s = drv_rtc[DRV_ID_RTC]->Init()) { return s; }
    IF_STATUS(s = GPIO_Init_()) { return s; }
    IF_STATUS(s = drv_gpio_v[DRV_ID_GPIOC]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = drv_gpio_v[DRV_ID_GPIOF]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = NVIC_Init_()) { return s; }
    IF_STATUS(s = drv_nvic_v[DRV_ID_NVIC]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = DMA_Init_()) { return s; }
    IF_STATUS(s = drv_dma_v[DRV_ID_DMA2]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = POWER_Init_()) { return s; }
#ifdef DATA_IN_ExtSRAM
    IF_STATUS(s = MEM_EXT_Init_()) { return s; }
    IF_STATUS(s = drv_mem_ext_v[DRV_ID_MEM_EXT_SRAM512K]->Init(OS_NULL)) { return s; }
#endif // DATA_IN_ExtSRAM
    IF_STATUS(s = TIMER_Init_()) { return s; }
    IF_STATUS(s = drv_timer_v[DRV_ID_TIMER5]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = drv_timer_v[DRV_ID_TIMER10]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = SDIO__Init()) { return s; }
#if (1 == USBH_ENABLED)
    IF_STATUS(s = USBH__Init()) { return s; }
#endif // USBH_ENABLED
    // High-level drivers init.
    IF_STATUS(s = LED_Init_()) { return s; }
//    IF_STATUS(s = drv_led[DRV_ID_LED_PULSE]->Init(OS_NULL)) { return s; }
//    IF_STATUS(s = drv_led[DRV_ID_LED_FS]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = drv_led_v[DRV_ID_LED_ASSERT]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = drv_led_v[DRV_ID_LED_USER]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = BUTTON_Init_()) { return s; }
    IF_STATUS(s = drv_button_v[DRV_ID_BUTTON_WAKEUP]->Init(OS_NULL)) { return s; }
    IF_STATUS(s = drv_button_v[DRV_ID_BUTTON_TAMPER]->Init(OS_NULL)) { return s; }
    return s;
}
