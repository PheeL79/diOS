/**************************************************************************//**
* @file    drv_gpio.c
* @brief   GPIO driver.
* @author  A. Filyanov
******************************************************************************/
#define _DRV_GPIO_C_

#include "hal.h"
#include "os_debug.h"
#include "os_supervise.h"
#include "os_network.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_gpio"

//-----------------------------------------------------------------------------
typedef struct {
    OS_QueueHd      slot_qhd;
    OS_SignalId     signal_id;
    OS_SignalData   signal_data;
    U16             owners;
} DrvGpioConfigDyn;

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

ALIGN_BEGIN static DrvGpioConfigDyn gpio_dyn_v[16] ALIGN_END;

/*****************************************************************************/
Status GPIO_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_gpio_v, 0x0, sizeof(drv_gpio_v));
    HAL_MemSet(gpio_dyn_v, 0x0, sizeof(gpio_dyn_v));
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
    //Set default pin state before actual output enabled.
    if ((GPIO_MODE_OUTPUT_PP == init_stc_p->init.Mode) ||
        (GPIO_MODE_OUTPUT_OD == init_stc_p->init.Mode)) {
        GPIO_Write((void*)gpio, 0, (void*)OFF);
    }
    HAL_GPIO_Init(init_stc_p->port, (GPIO_InitTypeDef*)&init_stc_p->init);
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_SetPriority(init_stc_p->exti.irq, init_stc_p->exti.nvic_prio_pre, init_stc_p->exti.nvic_prio_sub);
    }
    if (HAL_NULL != init_stc_p->timer_hd_p) {
        switch ((Int)init_stc_p->timer_itf) {
            case (Int)TIM1:
                __TIM1_CLK_ENABLE();
                break;
            case (Int)TIM2:
                __TIM2_CLK_ENABLE();
                break;
            case (Int)TIM3:
                __TIM3_CLK_ENABLE();
                break;
            case (Int)TIM4:
                __TIM4_CLK_ENABLE();
                break;
            case (Int)TIM5:
                __TIM5_CLK_ENABLE();
                break;
            case (Int)TIM6:
                __TIM6_CLK_ENABLE();
                break;
            case (Int)TIM7:
                __TIM7_CLK_ENABLE();
                break;
            case (Int)TIM8:
                __TIM8_CLK_ENABLE();
                break;
            case (Int)TIM9:
                __TIM9_CLK_ENABLE();
                break;
            case (Int)TIM10:
                __TIM10_CLK_ENABLE();
                break;
            case (Int)TIM11:
                __TIM11_CLK_ENABLE();
                break;
            case (Int)TIM12:
                __TIM12_CLK_ENABLE();
                break;
            case (Int)TIM13:
                __TIM13_CLK_ENABLE();
                break;
            case (Int)TIM14:
                __TIM14_CLK_ENABLE();
                break;
            default:
                s = S_INVALID_TIMER;
                break;
        }
        init_stc_p->timer_hd_p->Instance = init_stc_p->timer_itf;
        init_stc_p->timer_hd_p->Init = init_stc_p->timer_init;
        if (HAL_OK == HAL_TIM_PWM_Init(init_stc_p->timer_hd_p)) {
            if (HAL_OK == HAL_TIM_PWM_ConfigChannel(init_stc_p->timer_hd_p, (TIM_OC_InitTypeDef*)&init_stc_p->timer_oc, init_stc_p->timer_channel)) {
            } else { s = S_DRIVER_ERROR; }
        } else { s = S_DRIVER_ERROR; }
    }
    return s;
}

/*****************************************************************************/
Status GPIO_DeInit(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    if (HAL_NULL != init_stc_p->timer_itf) {
        if (HAL_OK == HAL_TIM_PWM_DeInit(init_stc_p->timer_hd_p)) {
//TODO(A. Filyanov) Disable timer peripheral clock!
        } else { s = S_DRIVER_ERROR; }
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
        case DRV_REQ_GPIO_PWM_SET: {
            const DrvGpioArgsIoCtlPwm* io_pwm_args_p = (DrvGpioArgsIoCtlPwm*)args_p;
            const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[io_pwm_args_p->gpio];
                s = S_OK;
                OS_ASSERT_DEBUG(OS_NULL != init_stc_p->timer_hd_p);
                __HAL_TIM_SET_COMPARE(init_stc_p->timer_hd_p, init_stc_p->timer_channel, io_pwm_args_p->pwm_pulse);
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
        case DRV_REQ_GPIO_OPEN: {
                s = S_OK;
                if (HAL_NULL != args_p) {
                    const DrvGpioArgsIoCtlOpen* open_args_p = (DrvGpioArgsIoCtlOpen*)args_p;
                    const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[open_args_p->gpio];
                    const U32 gpio_pin = Bit2PositionGet(gpio_v[open_args_p->gpio].init.Pin);
                    DrvGpioConfigDyn* gpio_dyn_p = (DrvGpioConfigDyn*)&gpio_dyn_v[gpio_pin];

                    if (0 != init_stc_p->exti.irq) {
                        gpio_dyn_p->signal_id   = open_args_p->signal_id;
                        gpio_dyn_p->signal_data = open_args_p->signal_data;
                        gpio_dyn_p->slot_qhd    = open_args_p->slot_qhd;

                        __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
                        HAL_NVIC_EnableIRQ(init_stc_p->exti.irq);
                    }
                    if (HAL_NULL != init_stc_p->timer_hd_p) {
                        if (HAL_OK == HAL_TIM_PWM_Start(init_stc_p->timer_hd_p, init_stc_p->timer_channel)) {
                        } else { s = S_DRIVER_ERROR; }
                    }
                    IF_OK(s) {
                        gpio_dyn_p->owners++;
                    }
                }
            }
            break;
        case DRV_REQ_GPIO_CLOSE: {
            const Gpio gpio = (Gpio)args_p;
            const U32 gpio_pin = Bit2PositionGet(gpio_v[gpio].init.Pin);
            DrvGpioConfigDyn* gpio_dyn_p = (DrvGpioConfigDyn*)&gpio_dyn_v[gpio_pin];
                s = S_OK;
                if (1 == gpio_dyn_p->owners) {
                    const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
                    if (0 != init_stc_p->exti.irq) {
                        HAL_NVIC_DisableIRQ(init_stc_p->exti.irq);
                    }
                    if (HAL_NULL != init_stc_p->timer_hd_p) {
                        if (HAL_OK == HAL_TIM_PWM_Stop(init_stc_p->timer_hd_p, init_stc_p->timer_channel)) {
                        } else { s = S_DRIVER_ERROR; }
                    }
                    gpio_dyn_p->slot_qhd = OS_NULL;
                }
                IF_OK(s) {
                    if (0 < gpio_dyn_p->owners) {
                        gpio_dyn_p->owners--;
                    } else {
                        s = S_CLOSED;
                        HAL_LOG_S(L_WARNING, s);
                    }
                }
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
}

// IRQ handlers-----------------------------------------------------------------
/******************************************************************************/
INLINE static void ISR_Handler(const OS_QueueHd slot_qhd, const OS_SignalId id, const OS_SignalData data);
void ISR_Handler(const OS_QueueHd slot_qhd, const OS_SignalId id, const OS_SignalData data)
{
const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_GPIO, id, data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(slot_qhd, signal, OS_MSG_PRIO_HIGH));
}

//TODO(A.Filyanov) Macro expansion functions generator.
/******************************************************************************/
void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void)
{
static const Int gpio_pin = 0;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI1_IRQHandler(void);
void EXTI1_IRQHandler(void)
{
static const Int gpio_pin = 1;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI2_IRQHandler(void);
void EXTI2_IRQHandler(void)
{
static const Int gpio_pin = 2;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI3_IRQHandler(void);
void EXTI3_IRQHandler(void)
{
static const Int gpio_pin = 3;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI4_IRQHandler(void);
void EXTI4_IRQHandler(void)
{
static const Int gpio_pin = 4;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI9_5_IRQHandler(void);
void EXTI9_5_IRQHandler(void)
{
static const Int gpio_pin = 5;
//TODO(A.Filyanov) Test asserted IRQ lines for actual pins numbers.
    __HAL_GPIO_EXTI_CLEAR_IT(BF_PREP(UINT_MAX, gpio_pin, (9 - (gpio_pin - 1))));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}

/******************************************************************************/
void EXTI15_10_IRQHandler(void);
void EXTI15_10_IRQHandler(void)
{
static const Int gpio_pin = 10;
//TODO(A.Filyanov) Test asserted IRQ lines for actual pins numbers.
    __HAL_GPIO_EXTI_CLEAR_IT(BF_PREP(UINT_MAX, gpio_pin, (15 - (gpio_pin - 1))));
    ISR_Handler(gpio_dyn_v[gpio_pin].slot_qhd, gpio_dyn_v[gpio_pin].signal_id, gpio_dyn_v[gpio_pin].signal_data);
}