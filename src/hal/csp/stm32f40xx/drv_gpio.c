/**************************************************************************//**
* @file    drv_gpio.c
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_debug.h"
#include "os_supervise.h"
#include "os_network.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_gpio"

//-----------------------------------------------------------------------------
/// @brief      GPIO initialize.
/// @return     #Status.
Status          GPIO_Init_(void);
static Status   GPIO_Init(void* args_p);
static Status   GPIO_DeInit(void* args_p);
static Status   GPIO_LL_Init(void* args_p);
//static Status   GPIO_LL_DeInit(void* args_p);
static Status   GPIO_Open(void* args_p);
static Status   GPIO_Close(void* args_p);
static Status   GPIO_Read(void* data_in_p, Size size, void* args_p);
static Status   GPIO_Write(void* data_out_p, Size size, void* args_p);
static Status   GPIO_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
ALIGN_BEGIN HAL_DriverItf* drv_gpio_v[DRV_ID_GPIO_LAST] ALIGN_END;
static ConstStrP gpio_str_v[] = {
    FOREACH_GPIO(GENERATE_STRING)
};

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_gpio = {
    .Init   = GPIO_Init,
    .DeInit = GPIO_DeInit,
    .Open   = GPIO_Open,
    .Close  = GPIO_Close,
    .Read   = GPIO_Read,
    .Write  = GPIO_Write,
    .IoCtl  = GPIO_IoCtl
};

//GPIO
ALIGN_BEGIN static const HAL_GPIO_InitStruct gpio_v[] ALIGN_END = {
    [GPIO_DEBUG_1] = {
        .port               = HAL_GPIO_DEBUG_1_PORT,
        .init = {
            .Pin            = HAL_GPIO_DEBUG_1_PIN,
            .Mode           = HAL_GPIO_DEBUG_1_MODE,
            .Pull           = HAL_GPIO_DEBUG_1_PULL,
            .Speed          = HAL_GPIO_DEBUG_1_SPEED,
            .Alternate      = HAL_GPIO_DEBUG_1_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_DEBUG_2] = {
        .port               = HAL_GPIO_DEBUG_2_PORT,
        .init = {
            .Pin            = HAL_GPIO_DEBUG_2_PIN,
            .Mode           = HAL_GPIO_DEBUG_2_MODE,
            .Pull           = HAL_GPIO_DEBUG_2_PULL,
            .Speed          = HAL_GPIO_DEBUG_2_SPEED,
            .Alternate      = HAL_GPIO_DEBUG_2_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_ASSERT] = {
        .port               = HAL_GPIO_ASSERT_PORT,
        .init = {
            .Pin            = HAL_GPIO_ASSERT_PIN,
            .Mode           = HAL_GPIO_ASSERT_MODE,
            .Pull           = HAL_GPIO_ASSERT_PULL,
            .Speed          = HAL_GPIO_ASSERT_SPEED,
            .Alternate      = HAL_GPIO_ASSERT_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_PULSE] = {
        .port               = HAL_GPIO_LED_PULSE_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_PULSE_PIN,
            .Mode           = GPIO_MODE_OUTPUT_PP,
            .Pull           = GPIO_NOPULL,
            .Speed          = GPIO_SPEED_LOW,
            .Alternate      = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_FS] = {
        .port               = HAL_GPIO_LED_FS_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_FS_PIN,
            .Mode           = GPIO_MODE_OUTPUT_PP,
            .Pull           = GPIO_NOPULL,
            .Speed          = GPIO_SPEED_LOW,
            .Alternate      = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_ASSERT] = {
        .port               = HAL_GPIO_LED_ASSERT_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_ASSERT_PIN,
            .Mode           = GPIO_MODE_OUTPUT_PP,
            .Pull           = GPIO_NOPULL,
            .Speed          = GPIO_SPEED_LOW,
            .Alternate      = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_USER] = {
        .port               = HAL_GPIO_LED_USER_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_USER_PIN,
            .Mode           = GPIO_MODE_OUTPUT_PP,
            .Pull           = GPIO_NOPULL,
            .Speed          = GPIO_SPEED_LOW,
            .Alternate      = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_BUTTON_WAKEUP] = {
        .port               = HAL_GPIO_BUTTON_WAKEUP_PORT,
        .init = {
            .Pin            = HAL_GPIO_BUTTON_WAKEUP_PIN,
            .Mode           = HAL_GPIO_BUTTON_WAKEUP_MODE,
            .Pull           = HAL_GPIO_BUTTON_WAKEUP_PULL,
            .Speed          = HAL_GPIO_BUTTON_WAKEUP_SPEED,
            .Alternate      = HAL_GPIO_BUTTON_WAKEUP_ALT
        },
        .exti = {
            .irq            = HAL_GPIO_BUTTON_WAKEUP_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_BUTTON_WAKEUP,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_BUTTON_TAMPER] = {
        .port               = HAL_GPIO_BUTTON_TAMPER_PORT,
        .init = {
            .Pin            = HAL_GPIO_BUTTON_TAMPER_PIN,
            .Mode           = HAL_GPIO_BUTTON_TAMPER_MODE,
            .Pull           = HAL_GPIO_BUTTON_TAMPER_PULL,
            .Speed          = HAL_GPIO_BUTTON_TAMPER_SPEED,
            .Alternate      = HAL_GPIO_BUTTON_TAMPER_ALT
        },
        .exti = {
            .irq            = HAL_GPIO_BUTTON_TAMPER_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_BUTTON_TAMPER,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
};

/*****************************************************************************/
Status GPIO_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_gpio_v, 0x0, sizeof(drv_gpio_v));
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_gpio_v, drv_gpio_v[0]); ++i) {
        drv_gpio_v[i] = &drv_gpio;
        if (OS_NULL != drv_gpio_v[i]) {
            for (Size j = 0; j < ITEMS_COUNT_GET(gpio_v, gpio_v[0]); ++j) {
                IF_STATUS(s = drv_gpio_v[i]->Init((void*)j)) {
                   break;
                }
            }
        }
    }
    return s;
}

/*****************************************************************************/
Status GPIO_Init(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
Status s = S_OK;
    HAL_LOG(L_INFO, "Init: %s ", gpio_str_v[gpio]);
    s = GPIO_LL_Init(args_p);
    HAL_TRACE_S(L_INFO, s);
    return s;
}

/*****************************************************************************/
Status GPIO_LL_Init(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    /* Enable the GPIO clock */
    switch ((Int)init_stc_p->port) {
        case (Int)GPIOA:
            __GPIOA_CLK_ENABLE();
            break;
        case (Int)GPIOB:
            __GPIOB_CLK_ENABLE();
            break;
        case (Int)GPIOC:
            __GPIOC_CLK_ENABLE();
            break;
        case (Int)GPIOD:
            __GPIOD_CLK_ENABLE();
            break;
        case (Int)GPIOE:
            __GPIOE_CLK_ENABLE();
            break;
        case (Int)GPIOF:
            __GPIOF_CLK_ENABLE();
            break;
        case (Int)GPIOH:
            __GPIOH_CLK_ENABLE();
            break;
        default: break;
    }
    HAL_GPIO_Init(init_stc_p->port, (GPIO_InitTypeDef*)&init_stc_p->init);
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_SetPriority(init_stc_p->exti.irq, init_stc_p->exti.nvic_prio_pre, init_stc_p->exti.nvic_prio_sub);
        HAL_NVIC_EnableIRQ(init_stc_p->exti.irq);
    }
    return s;
}

/*****************************************************************************/
Status GPIO_DeInit(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_DisableIRQ(init_stc_p->exti.irq);
    }
    if ((GPIO_MODE_OUTPUT_PP == init_stc_p->init.Mode) ||
        (GPIO_MODE_OUTPUT_OD == init_stc_p->init.Mode)) {
        GPIO_Write((void*)gpio, 0, (void*)OFF);
    }
    HAL_GPIO_DeInit(init_stc_p->port, init_stc_p->init.Pin);
    return s;
}

/*****************************************************************************/
Status GPIO_Open(void* args_p)
{
Status s = S_OK;
    return s;
}

/*****************************************************************************/
Status GPIO_Close(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status GPIO_Read(void* data_in_p, Size size, void* args_p)
{
const Gpio gpio = (Gpio)data_in_p;
    if (sizeof(State) > size) { return S_INVALID_SIZE; }
    *(State*)data_in_p = (State)HAL_GPIO_ReadPin(gpio_v[gpio].port, gpio_v[gpio].init.Pin);
    return S_OK;
}

/******************************************************************************/
Status GPIO_Write(void* data_out_p, Size size, void* args_p)
{
const Gpio gpio = (Gpio)data_out_p;
const GPIO_PinState state = (GPIO_PinState)((UInt)gpio_v[gpio].is_inverted ^ (UInt)args_p);
    (void)size;
    HAL_GPIO_WritePin(gpio_v[gpio].port, gpio_v[gpio].init.Pin, state);
    return S_OK;
}

/******************************************************************************/
Status GPIO_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_SLEEP:
                case PWR_STOP:
                case PWR_STANDBY:
                case PWR_HIBERNATE:
                case PWR_SHUTDOWN:
                    for (Size i = 0; i < ITEMS_COUNT_GET(drv_gpio_v, drv_gpio_v[0]); ++i) {
                        if (OS_NULL != drv_gpio_v[i]) {
                            for (Size j = 0; j < ITEMS_COUNT_GET(gpio_v, gpio_v[0]); ++j) {
                                IF_STATUS(s = drv_gpio_v[i]->Write((void*)j, 0, (void*)OFF)) {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case DRV_REQ_GPIO_EXTI_IRQ_ENABLE: {
                const Int gpio = (Int)args_p;
                HAL_NVIC_EnableIRQ(gpio_v[gpio].exti.irq);
                s = S_OK;
            }
            break;
        case DRV_REQ_GPIO_EXTI_IRQ_DISABLE: {
                const Int gpio = (Int)args_p;
                HAL_NVIC_DisableIRQ(gpio_v[gpio].exti.irq);
                s = S_OK;
            }
            break;
        case DRV_REQ_GPIO_TOGGLE: {
                const Int gpio = (Int)args_p;
                HAL_GPIO_TogglePin(gpio_v[gpio].port, gpio_v[gpio].init.Pin);
                s = S_OK;
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
}

/*****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
        case GPIO_PIN_0:
            {
            extern HAL_IrqCallbackFunc wakeup_irq_callback_func;
            if (OS_NULL != wakeup_irq_callback_func) {
                wakeup_irq_callback_func();
            }
            }
            break;
        case GPIO_PIN_3:
#if (HAL_ETH_ENABLED)
            {
            extern OS_QueueHd netd_stdin_qhd;
            if (OS_NULL != netd_stdin_qhd) {
                const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_ETH0, OS_SIG_ETH_LINK_STATE_CHANGED, 0);
                OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(netd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
            }
            }
#endif //(HAL_ETH_ENABLED)
            break;
        case GPIO_PIN_13:
            {
            extern HAL_IrqCallbackFunc tamper_irq_callback_func;
            if (OS_NULL != tamper_irq_callback_func) {
                tamper_irq_callback_func();
            }
            }
            break;
        default:
            OS_ASSERT(OS_FALSE);
            break;
    }
}

// IRQ handlers-----------------------------------------------------------------
/******************************************************************************/
void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/******************************************************************************/
void EXTI3_IRQHandler(void);
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}