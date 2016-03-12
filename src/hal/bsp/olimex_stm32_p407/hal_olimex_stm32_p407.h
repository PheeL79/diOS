/**************************************************************************//**
* @file    hal_olimex_stm32_p407.h
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_OLIMEX_STM32_P407_H_
#define _HAL_OLIMEX_STM32_P407_H_

#include "stm32f4xx_hal.h"
#include "hal_config.h"
#include "common.h"

//-----------------------------------------------------------------------------
#define HAL_IO                          volatile

#define HAL_CRITICAL_SECTION_ENTER()    __disable_irq()
#define HAL_CRITICAL_SECTION_EXIT()     __enable_irq()

#define HAL_SYSTICK_START()             HAL_ResumeTick()
#define HAL_SYSTICK_STOP()              HAL_SuspendTick()

// Joseph Yiu's method
// From http://forums.arm.com/index.php?showtopic=13949
#define    DWT_CYCCNT                   *(HAL_IO U32*)0xE0001004
#define    DWT_CONTROL                  *(HAL_IO U32*)0xE0001000
#define    SCB_DEMCR                    *(HAL_IO U32*)0xE000EDFC
extern int cycles_diff;
extern int cycles_last;

#define HAL_CORE_CYCLES                 DWT_CYCCNT
#define HAL_DWT_STOPWATCH_START         { cycles_last = HAL_CORE_CYCLES; }
#define HAL_DWT_STOPWATCH_STOP          { cycles_diff = HAL_CORE_CYCLES - cycles_last; }

extern U32 SystemCoreClockKHz;
///@brief
///@details Example:
///         DWT_STOPWATCH_START;
///         HAL_DelayMs(100);
///         DWT_STOPWATCH_STOP;
///         D_LOG(L_DEBUG_1, "%d(ms)", CYCLES_TO_MS(cycles_diff));
#define CYCLES_TO_MS(cycles)            ((U32)((cycles) / SystemCoreClockKHz))

typedef enum {
    HAL_EXCEPT_UNDEF,
    HAL_EXCEPT_NMI,
    HAL_EXCEPT_HARD_FAULT,
    HAL_EXCEPT_MEM_MANAGE,
    HAL_EXCEPT_BUS_FAULT,
    HAL_EXCEPT_USAGE_FAULT,
    HAL_EXCEPT_SVC,
    HAL_EXCEPT_DEBUG_MON,
    HAL_EXCEPT_PEND_SV,
    HAL_EXCEPT_WWDG,
    HAL_EXCEPT_PVD
} HAL_Exception;

//GPIO
typedef struct {
    IRQn_Type               irq;
    U32                     nvic_prio_pre;
    U32                     nvic_prio_sub;
} HAL_EXTI_InitStruct;

typedef struct {
    GPIO_TypeDef*           port;
    GPIO_InitTypeDef        init;
    HAL_EXTI_InitStruct     exti;
    TIM_HandleTypeDef*      timer_hd_p;         ///< for PWM mode.
    Bool                    is_inverted;
} HAL_GPIO_InitStruct;

#define FOREACH_GPIO(GPIO)       \
        GPIO(GPIO_DEBUG_1)       \
        GPIO(GPIO_DEBUG_2)       \
        GPIO(GPIO_ASSERT)        \
        GPIO(GPIO_LED_PULSE)     \
        GPIO(GPIO_LED_FS)        \
        GPIO(GPIO_LED_ASSERT)    \
        GPIO(GPIO_LED_USER)      \
        GPIO(GPIO_BUTTON_WAKEUP) \
        GPIO(GPIO_BUTTON_TAMPER) \
        GPIO(GPIO_ETH_MDINT)     \
        GPIO(GPIO_LAST)          \

typedef enum {
    FOREACH_GPIO(GENERATE_ENUM)
} Gpio;

//#ifndef _HAL_OLIMEX_STM32_P407_H_
//extern
//#endif
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
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_FS] = {
        .port               = HAL_GPIO_LED_FS_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_FS_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_ASSERT] = {
        .port               = HAL_GPIO_LED_ASSERT_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_ASSERT_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_USER] = {
        .port               = HAL_GPIO_LED_USER_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_USER_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
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
    [GPIO_ETH_MDINT] = {
        .port               = HAL_ETH_GPIO_MDINT_PORT,
        .init = {
            .Pin            = HAL_ETH_GPIO_MDINT_PIN,
            .Mode           = HAL_ETH_GPIO_MDINT_MODE,
            .Pull           = HAL_ETH_GPIO_MDINT_PULL,
            .Speed          = HAL_ETH_GPIO_MDINT_SPEED,
            .Alternate      = HAL_ETH_GPIO_MDINT_ALT
        },
        .exti = {
            .irq            = HAL_ETH_GPIO_MDINT_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_ETH0_MDINT,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
};

//CSP
#include "drv_crc.h"
#include "drv_adc.h"
#include "drv_spi.h"
#include "drv_i2s.h"
#include "drv_dma.h"
#include "drv_rtc.h"
#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_usart.h"
#include "drv_power.h"
#include "drv_usbh.h"
#include "drv_usbd.h"
#include "drv_eth.h"
//BSP
#include "drv_mem_ext.h"
#include "drv_trimmer.h"
#include "drv_button.h"
//#include "drv_audio_bsp.h"
#include "drv_media.h"
#include "drv_media_bsp.h"

//-----------------------------------------------------------------------------
/// @brief      Init HAL.
/// @return     #Status.
Status          HAL_Init_(void);

/// @brief      Init HAL BSP.
/// @return     #Status.
Status          HAL_BSP_Init(void);

/// @brief      Delay (ms).
/// @param[in]  delay_ms        Delay value (ms).
/// @return     None.
void            HAL_DelayMs(U32 delay_ms);

/// @brief      Delay (CPU cycles).
/// @param[in]  cycles          Delay value (CPU cycles).
/// @return     None.
void            HAL_DelayCycles(U32 delay_cycles);

/// @brief      Count bits.
/// @param[in]  value           Input value.
/// @return     Bit count.
U8              HAL_BitCount(U32 value);

/// @brief      Reverse bits.
/// @param[in]  value           Input value.
/// @return     Bit reversed value.
U32             HAL_BitReverse(register U32 value);

/// @brief      Reverse bytes.
/// @param[in]  value           Input value.
/// @return     Byte reversed value.
/// @note       Converts 0xA1B1C1D1 value into 0xD1C1B1A1.
U32             HAL_ByteReverse(register U32 value);

/// @brief      Converts a 2 digit decimal to BCD format.
/// @param[in]  value           Byte to be converted.
/// @return     Converted byte.
U8              HAL_ByteToBcd2(U8 value);

/// @brief      Convert from 2 digit BCD to Binary.
/// @param[in]  value           BCD value to be converted.
/// @return     Converted word.
U8              HAL_Bcd2ToByte(U8 value);

/// @brief      Soft reset.
/// @return     None.
void            HAL_SoftReset(void);

/// @brief      Get system core clock value.
/// @return     System core clock value (Hz).
U32             HAL_SystemCoreClockGet(void);

/// @brief      Clear standart I/O shell stream.
/// @return     None.
void            HAL_StdIoCls(void);

/// @brief      Get device description.
/// @param[out] dev_desc_p      Device description.
/// @param[in]  size            Description size.
/// @return     #Status.
Status          HAL_DeviceDescriptionGet(DeviceDesc* dev_desc_p, const U16 size);

/// @brief      Handle the hardware exception.
/// @return     None.
void            HAL_ExceptionHandler(const HAL_Exception exp);

#endif // _HAL_OLIMEX_STM32_P407_H_
