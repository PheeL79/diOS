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

static Status BUTTON_WakeupInit(void);
static Status BUTTON_NVIC_WakeupInit(void);
static Status BUTTON_WakeupDeInit(void);
static Status BUTTON_WakeupOpen(void* args_p);
static Status BUTTON_WakeupClose(void);
static Status BUTTON_WakeupIoCtl(const U32 request_id, void* args_p);

static Status BUTTON_TamperInit(void);
static Status BUTTON_NVIC_TamperInit(void);
static Status BUTTON_TamperDeInit(void);
static Status BUTTON_TamperOpen(void* args_p);
static Status BUTTON_TamperClose(void);
static Status BUTTON_TamperIoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_button_v[DRV_ID_BUTTON_LAST];

static HAL_IrqCallbackFunc wakeup_irq_callback_func = OS_NULL;
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
Status s = S_OK;
    memset(drv_button_v, 0x0, sizeof(drv_button_v));
    drv_button_v[DRV_ID_BUTTON_WAKEUP] = &drv_button_wakeup;
    drv_button_v[DRV_ID_BUTTON_TAMPER] = &drv_button_tamper;
    return s;
}

/*****************************************************************************/
Status BUTTON_WakeupInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
Status s = S_OK;

    D_LOG(D_INFO, "Wakeup init: ");
    OS_CriticalSectionEnter(); {
        GPIO_StructInit(&GPIO_InitStructure);
        /* Enable the GPIOA clock */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
        s = BUTTON_NVIC_WakeupInit();
    } OS_CriticalSectionExit();
    D_TRACE_S(D_INFO, s);
    return s;
}

/*****************************************************************************/
Status BUTTON_NVIC_WakeupInit(void)
{
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

    D_LOG(D_INFO, "NVIC Wakeup Init: ");
    EXTI_StructInit(&EXTI_InitStructure);
    /* Enable The external line0 interrupt */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_StructInit(&NVIC_InitStructure);
    // EXTI line 0 IRQ channel configuration
    NVIC_InitStructure.NVIC_IRQChannel                  = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line0);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupDeInit(void)
{
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupOpen(void* args_p)
{
    PWR_WakeUpPinCmd(ENABLE);
    wakeup_irq_callback_func = ((HAL_IrqCallbackFunc)args_p);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_WakeupClose(void)
{
    PWR_WakeUpPinCmd(DISABLE);
    wakeup_irq_callback_func = OS_NULL;
    return S_OK;
}

/******************************************************************************/
Status BUTTON_WakeupIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                default:
                    break;
            }
            break;
        default:
            D_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

/*****************************************************************************/
Status BUTTON_TamperInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
Status s = S_OK;

    D_LOG(D_INFO, "Tamper init: ");
    OS_CriticalSectionEnter(); {
        GPIO_StructInit(&GPIO_InitStructure);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource13, GPIO_AF_TAMPER);
        s = BUTTON_NVIC_TamperInit();
        /* Disable the Tamper 1 detection */
        RTC_TamperCmd(RTC_Tamper_1, DISABLE);
        /* Configure the Tamper 1 Trigger */
        RTC_TamperTriggerConfig(RTC_Tamper_1, RTC_TamperTrigger_RisingEdge);
        /* Enable the Tamper interrupt */
        RTC_ITConfig(RTC_IT_TAMP, ENABLE);
        /* Clear Tamper 1 pin Event(TAMP1F) pending flag */
        RTC_ClearFlag(RTC_FLAG_TAMP1F);
        /* Clear Tamper 1 pin interrupt pending bit */
        EXTI_ClearITPendingBit(EXTI_Line13);
        EXTI_ClearITPendingBit(EXTI_Line21);
        RTC_ClearITPendingBit(RTC_IT_TAMP1);
        /* Enable the Tamper 1 detection */
        RTC_TamperCmd(RTC_Tamper_1, ENABLE);
    } OS_CriticalSectionExit();
    D_TRACE_S(D_INFO, s);
    return s;
}

/*****************************************************************************/
Status BUTTON_NVIC_TamperInit(void)
{
EXTI_InitTypeDef  EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

    D_LOG(D_INFO, "NVIC Tamper Init: ");
    /* Enable The external line21 interrupt */
    EXTI_StructInit(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line    = EXTI_Line21;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_StructInit(&NVIC_InitStructure);
    // RTC Tamper IRQ channel configuration
    NVIC_InitStructure.NVIC_IRQChannel                  = TAMP_STAMP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_TamperDeInit(void)
{
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_TamperOpen(void* args_p)
{
    tamper_irq_callback_func = ((HAL_IrqCallbackFunc)args_p);
    return S_OK;
}

/*****************************************************************************/
Status BUTTON_TamperClose(void)
{
    tamper_irq_callback_func = OS_NULL;
    return S_OK;
}

/******************************************************************************/
Status BUTTON_TamperIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                default:
                    break;
            }
            break;
        default:
            D_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

// Buttons IRQ handlers --------------------------------------------------------
/******************************************************************************/
void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void)
{
    if (RESET != EXTI_GetITStatus(EXTI_Line0)) {
        EXTI_ClearITPendingBit(EXTI_Line0);
        if (OS_NULL != wakeup_irq_callback_func) {
            wakeup_irq_callback_func();
        }
    }
}

/******************************************************************************/
void TAMP_STAMP_IRQHandler(void);
void TAMP_STAMP_IRQHandler(void)
{
    if (RESET != RTC_GetFlagStatus(RTC_FLAG_TAMP1F)) {
        /* Clear Tamper 1 pin Event pending flag */
        RTC_ClearFlag(RTC_FLAG_TAMP1F);
        EXTI_ClearITPendingBit(EXTI_Line21);
        RTC_ClearITPendingBit(RTC_IT_TAMP1);
        if (OS_NULL != tamper_irq_callback_func) {
            tamper_irq_callback_func();
        }
    }
}
