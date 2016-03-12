/**************************************************************************//**
* @file    drv_led.c
* @brief   LEDs driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_power.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_led"

//-----------------------------------------------------------------------------
/// @brief      Init LEDs.
/// @return     #Status.
Status          LED_Init_(void);
static Status   LED_Init(void* args_p);
static Status   LED_DeInit(void* args_p);
static Status   LED_LL_Init(void* args_p);
//static Status   LED_LL_DeInit(void* args_p);
static Status   LED_Open(void* args_p);
static Status   LED_Close(void* args_p);
static Status   LED_Read(void* data_in_p, Size size, void* args_p);
static Status   LED_Write(void* data_out_p, Size size, void* args_p);
static Status   LED_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
ALIGN_BEGIN HAL_DriverItf* drv_led_v[DRV_ID_LED_LAST] ALIGN_END;
static ConstStrP led_str_v[] = {
    FOREACH_LED(GENERATE_STRING)
};

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_led = {
    .Init   = LED_Init,
    .DeInit = LED_DeInit,
    .Open   = LED_Open,
    .Close  = LED_Close,
    .Read   = LED_Read,
    .Write  = LED_Write,
    .IoCtl  = LED_IoCtl
};

ALIGN_BEGIN static const HAL_LED_InitStruct led_v[] ALIGN_END = {
    [LED_PULSE] = {
        .gpio = {
            .port               = HAL_LED_LED_PULSE_PORT,
            .init = {
                .Pin            = HAL_LED_LED_PULSE_PIN,
                .Mode           = HAL_LED_LED_MODE,
                .Pull           = HAL_LED_LED_PULL,
                .Speed          = HAL_LED_LED_SPEED,
                .Alternate      = HAL_LED_LED_ALT
            },
            .is_inverted        = HAL_FALSE
        },
        .timer_hd_p             = HAL_NULL,
    },
    [LED_FS] = {
        .gpio = {
            .port               = HAL_LED_LED_FS_PORT,
            .init = {
                .Pin            = HAL_LED_LED_FS_PIN,
                .Mode           = HAL_LED_LED_MODE,
                .Pull           = HAL_LED_LED_PULL,
                .Speed          = HAL_LED_LED_SPEED,
                .Alternate      = HAL_LED_LED_ALT
            },
            .is_inverted        = HAL_FALSE
        },
        .timer_hd_p             = HAL_NULL,
    },
    [LED_ASSERT] = {
        .gpio = {
            .port               = HAL_LED_LED_ASSERT_PORT,
            .init = {
                .Pin            = HAL_LED_LED_ASSERT_PIN,
                .Mode           = HAL_LED_LED_MODE,
                .Pull           = HAL_LED_LED_PULL,
                .Speed          = HAL_LED_LED_SPEED,
                .Alternate      = HAL_LED_LED_ALT
            },
            .is_inverted        = HAL_FALSE
        },
        .timer_hd_p             = HAL_NULL,
    },
    [LED_USER] = {
        .gpio = {
            .port               = HAL_LED_LED_USER_PORT,
            .init = {
                .Pin            = HAL_LED_LED_USER_PIN,
                .Mode           = HAL_LED_LED_MODE,
                .Pull           = HAL_LED_LED_PULL,
                .Speed          = HAL_LED_LED_SPEED,
                .Alternate      = HAL_LED_LED_ALT
            },
            .is_inverted        = HAL_FALSE
        },
        .timer_hd_p             = HAL_NULL,
    },
};

/*****************************************************************************/
Status LED_Init_(void)
{
Status s = S_OK;
    HAL_MemSet(drv_led_v, 0x0, sizeof(drv_led_v));
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_led_v, drv_led_v[0]); ++i) {
        drv_led_v[i] = &drv_led;
        if (OS_NULL != drv_led_v[i]) {
            for (Size j = 0; j < ITEMS_COUNT_GET(gpio_v, gpio_v[0]); ++j) {
                IF_STATUS(s = drv_led_v[i]->Init((void*)j)) {
                   break;
                }
            }
        }
    }
    return s;
}
/*****************************************************************************/
Status LED_Init(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
Status s = S_OK;
    HAL_LOG(L_INFO, "Init: %s ", gpio_str_v[gpio]);
    s = LED_LL_Init(args_p);
    HAL_TRACE_S(L_INFO, s);
    return s;
}

/*****************************************************************************/
Status LED_LL_Init(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_LED_InitStruct* init_stc_p = (HAL_LED_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    /* Enable the LED clock */
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
    HAL_LED_Init(init_stc_p->port, (LED_InitTypeDef*)&init_stc_p->init);
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_SetPriority(init_stc_p->exti.irq, init_stc_p->exti.nvic_prio_pre, init_stc_p->exti.nvic_prio_sub);
        HAL_NVIC_EnableIRQ(init_stc_p->exti.irq);
    }
    return s;
}

/*****************************************************************************/
Status LED_DeInit(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_LED_InitStruct* init_stc_p = (HAL_LED_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_DisableIRQ(init_stc_p->exti.irq);
    }
    if ((LED_MODE_OUTPUT_PP == init_stc_p->init.Mode) ||
        (LED_MODE_OUTPUT_OD == init_stc_p->init.Mode)) {
        LED_Write((void*)gpio, 0, (void*)OFF);
    }
    HAL_LED_DeInit(init_stc_p->port, init_stc_p->init.Pin);
    return s;
}

/*****************************************************************************/
Status LED_Open(void* args_p)
{
Status s = S_OK;
    return s;
}

/*****************************************************************************/
Status LED_Close(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status LED_Read(void* data_in_p, Size size, void* args_p)
{
const Gpio gpio = (Gpio)data_in_p;
    if (sizeof(State) > size) { return S_INVALID_SIZE; }
    *(State*)data_in_p = (State)HAL_LED_ReadPin(gpio_v[gpio].port, gpio_v[gpio].init.Pin);
    return S_OK;
}

/******************************************************************************/
Status LED_Write(void* data_out_p, Size size, void* args_p)
{
const Gpio gpio = (Gpio)data_out_p;
const LED_PinState state = (LED_PinState)((UInt)gpio_v[gpio].is_inverted ^ (UInt)args_p);
    (void)size;
    HAL_LED_WritePin(gpio_v[gpio].port, gpio_v[gpio].init.Pin, state);
    return S_OK;
}

/******************************************************************************/
Status LED_IoCtl(const U32 request_id, void* args_p)
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
        case DRV_REQ_LED_EXTI_IRQ_ENABLE: {
                const Int gpio = (Int)args_p;
                HAL_NVIC_EnableIRQ(gpio_v[gpio].exti.irq);
                s = S_OK;
            }
            break;
        case DRV_REQ_LED_EXTI_IRQ_DISABLE: {
                const Int gpio = (Int)args_p;
                HAL_NVIC_DisableIRQ(gpio_v[gpio].exti.irq);
                s = S_OK;
            }
            break;
        case DRV_REQ_LED_TOGGLE: {
                const Int gpio = (Int)args_p;
                HAL_LED_TogglePin(gpio_v[gpio].port, gpio_v[gpio].init.Pin);
                s = S_OK;
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
}