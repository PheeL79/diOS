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
typedef struct {
    HAL_ISR_CallbackFunc    isr_callback_fp;
    void*                   args_p;
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

//TODO(A.Filyanov) Dynamic memory allocation for EXTI only GPIO!
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
//        HAL_TIM_PWM_ConfigChannel();
    }
    return s;
}

/*****************************************************************************/
Status GPIO_DeInit(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    HAL_GPIO_DeInit(init_stc_p->port, init_stc_p->init.Pin);
    return s;
}

/*****************************************************************************/
Status GPIO_Open(void* args_p)
{
Status s = S_OK;
    if (HAL_NULL != args_p) {
        const DrvGpioArgsOpen* open_args_p = (DrvGpioArgsOpen*)args_p;
        const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[open_args_p->gpio];
        const U32 gpio_pin = Bit2PositionGet(gpio_v[open_args_p->gpio].init.Pin);
        DrvGpioConfigDyn* gpio_dyn_p = (DrvGpioConfigDyn*)&gpio_dyn_v[gpio_pin];
        if (HAL_NULL != open_args_p->isr_callback_fp) {
            gpio_dyn_p->isr_callback_fp = open_args_p->isr_callback_fp;
            gpio_dyn_p->args_p = open_args_p->args_p;
        } else {
            s = S_INVALID_PTR;
        }
        if (0 != init_stc_p->exti.irq) {
            __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
            HAL_NVIC_EnableIRQ(init_stc_p->exti.irq);
        }
//        HAL_TIM_PWM_Start();
    }
    return s;
}

/*****************************************************************************/
Status GPIO_Close(void* args_p)
{
const Gpio gpio = (Gpio)args_p;
const HAL_GPIO_InitStruct* init_stc_p = (HAL_GPIO_InitStruct*)&gpio_v[gpio];
Status s = S_OK;
    if (0 != init_stc_p->exti.irq) {
        HAL_NVIC_DisableIRQ(init_stc_p->exti.irq);
    }
DrvGpioConfigDyn* gpio_dyn_p = (DrvGpioConfigDyn*)&gpio_dyn_v[gpio];
    gpio_dyn_p->isr_callback_fp = HAL_NULL;
//    HAL_TIM_PWM_Stop();
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
            const DrvGpioArgsIoCtlPwm* drv_args_p = (DrvGpioArgsIoCtlPwm*)args_p;
            TIM_HandleTypeDef* timer_hd_p = gpio_v[drv_args_p->gpio].timer_hd_p;
                if (HAL_NULL != timer_hd_p) {
//                    timer_hd_p->Init.
                } else {
                    s = S_INVALID_TIMER;
                }
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

// IRQ handlers-----------------------------------------------------------------
//TODO(A.Filyanov) Macro expansion functions generator.
/******************************************************************************/
void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void)
{
static const Int gpio_pin = 0;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI1_IRQHandler(void);
void EXTI1_IRQHandler(void)
{
static const Int gpio_pin = 1;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI2_IRQHandler(void);
void EXTI2_IRQHandler(void)
{
static const Int gpio_pin = 2;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI3_IRQHandler(void);
void EXTI3_IRQHandler(void)
{
static const Int gpio_pin = 3;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI4_IRQHandler(void);
void EXTI4_IRQHandler(void)
{
static const Int gpio_pin = 4;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI9_5_IRQHandler(void);
void EXTI9_5_IRQHandler(void)
{
static const Int gpio_pin = 5;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}

/******************************************************************************/
void EXTI15_10_IRQHandler(void);
void EXTI15_10_IRQHandler(void)
{
static const Int gpio_pin = 10;
    __HAL_GPIO_EXTI_CLEAR_IT(BIT(gpio_pin));
    HAL_ASSERT_DEBUG(HAL_NULL != gpio_dyn_v[gpio_pin].isr_callback_fp);
    gpio_dyn_v[gpio_pin].isr_callback_fp((void*)gpio_dyn_v[gpio_pin].args_p);
}