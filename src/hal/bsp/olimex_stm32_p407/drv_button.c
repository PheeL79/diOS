/**************************************************************************//**
* @file    drv_button.c
* @brief   Board buttons driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_supervise.h"
#include "os_power.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_button"

//-----------------------------------------------------------------------------
/// @brief   Buttons initialization.
/// @return  #Status.
Status BUTTON_Init_(void);

static Status BUTTON_WakeupInit(void* args_p);
static Status BUTTON_NVIC_WakeupInit(void);
static Status BUTTON_WakeupDeInit(void* args_p);
static Status BUTTON_WakeupOpen(void* args_p);
static Status BUTTON_WakeupClose(void* args_p);
static Status BUTTON_WakeupIoCtl(const U32 request_id, void* args_p);

static Status BUTTON_TamperInit(void* args_p);
static Status BUTTON_NVIC_TamperInit(void);
static Status BUTTON_TamperDeInit(void* args_p);
static Status BUTTON_TamperOpen(void* args_p);
static Status BUTTON_TamperClose(void* args_p);
static Status BUTTON_TamperIoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
extern RTC_HandleTypeDef rtc_handle;

HAL_DriverItf* drv_button_v[DRV_ID_BUTTON_LAST];

HAL_IrqCallbackFunc wakeup_irq_callback_func = OS_NULL;
static HAL_IrqCallbackFunc tamper_irq_callback_func = OS_NULL;

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_button_wakeup = {
    .Init   = BUTTON_WakeupInit,
    .DeInit = BUTTON_WakeupDeInit,
    .Open   = BUTTON_WakeupOpen,
    .Close  = BUTTON_WakeupClose,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = BUTTON_WakeupIoCtl
};

static HAL_DriverItf drv_button_tamper = {
    .Init   = BUTTON_TamperInit,
    .DeInit = BUTTON_TamperDeInit,
    .Open   = BUTTON_TamperOpen,
    .Close  = BUTTON_TamperClose,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = BUTTON_TamperIoCtl
};

/*****************************************************************************/
Status BUTTON_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_button_v, 0x0, sizeof(drv_button_v));
    drv_button_v[DRV_ID_BUTTON_WAKEUP] = &drv_button_wakeup;
    drv_button_v[DRV_ID_BUTTON_TAMPER] = &drv_button_tamper;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_button_v, drv_button_v[0]); ++i) {
        if (OS_NULL != drv_button_v[i]) {
            IF_STATUS(s = drv_button_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return s;
}

/*****************************************************************************/
Status BUTTON_WakeupInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;
Status s = S_OK;

    HAL_LOG(D_INFO, "Wakeup init: ");
    /* Disable all used wakeup sources: Pin1(PA.0) */
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    /* Clear all related wakeup flags */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __GPIOA_CLK_ENABLE();
    GPIO_InitStructure.Pin      = BUTTON_WAKEUP_PIN;
    GPIO_InitStructure.Mode     = GPIO_MODE_IT_RISING_FALLING | GPIO_MODE_EVT_RISING;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    s = BUTTON_NVIC_WakeupInit();
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/*****************************************************************************/
Status BUTTON_NVIC_WakeupInit(void)
{
    HAL_LOG(D_INFO, "NVIC Wakeup Init: ");
//    __HAL_GPIO_EXTI_CLEAR_FLAG(EXTI_IMR_MR0);
//    __HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR0);
    HAL_NVIC_SetPriority(EXTI0_IRQn, IRQ_PRIO_EXTI0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupDeInit(void* args_p)
{
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupOpen(void* args_p)
{
    wakeup_irq_callback_func = ((HAL_IrqCallbackFunc)args_p);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupClose(void* args_p)
{
    wakeup_irq_callback_func = OS_NULL;
    return S_OK;
}

/******************************************************************************/
Status BUTTON_WakeupIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                default:
                    s = S_OK;
                    break;
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

/*****************************************************************************/
Status BUTTON_TamperInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;
Status s = S_OK;

    HAL_LOG(D_INFO, "Tamper init: ");
    RTC_TamperTypeDef stamperstructure;

    __GPIOC_CLK_ENABLE();
    GPIO_InitStructure.Pin      = GPIO_PIN_13;
    GPIO_InitStructure.Mode     = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    GPIO_InitStructure.Alternate= GPIO_AF0_TAMPER;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Use PC13 as Tamper 1 with interrupt mode */
    stamperstructure.Filter                     = RTC_TAMPERFILTER_DISABLE;
    stamperstructure.PinSelection               = RTC_TAMPERPIN_PC13;
    stamperstructure.Tamper                     = RTC_TAMPER_1;
    stamperstructure.Trigger                    = RTC_TAMPERTRIGGER_RISINGEDGE;
    stamperstructure.SamplingFrequency          = RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256;
    stamperstructure.PrechargeDuration          = RTC_TAMPERPRECHARGEDURATION_1RTCCLK;
    stamperstructure.TamperPullUp               = RTC_TAMPER_PULLUP_DISABLE;
    stamperstructure.TimeStampOnTamperDetection = RTC_TIMESTAMPONTAMPERDETECTION_DISABLE;

    /* Clear the Tamper Flag */
    __HAL_RTC_TAMPER_CLEAR_FLAG(&rtc_handle, RTC_FLAG_TAMP1F);
    IF_STATUS(s = BUTTON_NVIC_TamperInit())     { return s; }
    if (HAL_OK != HAL_RTCEx_SetTamper_IT(&rtc_handle, &stamperstructure))   { return s = S_HARDWARE_FAULT; }
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/*****************************************************************************/
Status BUTTON_NVIC_TamperInit(void)
{
    HAL_LOG(D_INFO, "NVIC Wakeup Init: ");
    __HAL_GPIO_EXTI_CLEAR_FLAG(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT);
    __HAL_GPIO_EXTI_CLEAR_IT(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT);
    HAL_NVIC_SetPriority(TAMP_STAMP_IRQn, IRQ_PRIO_TAMP_STAMP, 0);
    HAL_NVIC_EnableIRQ(TAMP_STAMP_IRQn);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_TamperDeInit(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_TamperOpen(void* args_p)
{
Status s = S_OK;
    tamper_irq_callback_func = ((HAL_IrqCallbackFunc)args_p);
    return s;
}

/*****************************************************************************/
Status BUTTON_TamperClose(void* args_p)
{
Status s = S_OK;
    tamper_irq_callback_func = OS_NULL;
    return s;
}

/******************************************************************************/
Status BUTTON_TamperIoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                default:
                    s = S_OK;
                    break;
            }
            break;
        case DRV_REQ_BUTTON_TAMPER_ENABLE:
            s = BUTTON_TamperInit(OS_NULL);
            break;
        case DRV_REQ_BUTTON_TAMPER_DISABLE:
            if (HAL_OK == HAL_RTCEx_DeactivateTamper(&rtc_handle, RTC_TAMPER_1)) {
                s = S_OK;
            } else {
                s = S_HARDWARE_FAULT;
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

/******************************************************************************/
void HAL_RTCEx_Tamper1EventCallback(RTC_HandleTypeDef *hrtc)
{
    if (OS_NULL != tamper_irq_callback_func) {
        tamper_irq_callback_func();
    }
}

// Buttons IRQ handlers --------------------------------------------------------

/******************************************************************************/
void TAMP_STAMP_IRQHandler(void);
void TAMP_STAMP_IRQHandler(void)
{
    HAL_RTCEx_TamperTimeStampIRQHandler(&rtc_handle);
}
