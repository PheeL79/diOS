/**************************************************************************//**
* @file    hal_olimex_stm32_p407.h
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_OLIMEX_STM32_P407_H_
#define _HAL_OLIMEX_STM32_P407_H_

#define OLIMEX_STM32_P407

#include "stm32f4xx_hal.h"
#include "common.h"

//CSP
#include "drv_crc.h"
#include "drv_adc.h"
#include "drv_spi.h"
#include "drv_i2s.h"
#include "drv_dma.h"
#include "drv_rtc.h"
#include "drv_gpio.h"
#include "drv_nvic.h"
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
#include "drv_led.h"
//#include "drv_audio_bsp.h"
#include "drv_media.h"
#include "drv_media_bsp.h"

//-----------------------------------------------------------------------------
#define HAL_CRITICAL_SECTION_ENTER()    __disable_irq()
#define HAL_CRITICAL_SECTION_EXIT()     __enable_irq()

#define HAL_SYSTICK_START()             HAL_ResumeTick()
#define HAL_SYSTICK_STOP()              HAL_SuspendTick()

#define HAL_DEBUG_PIN1_CLK_ENABLE()     __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN1                  GPIO_PIN_4
#define HAL_DEBUG_PIN1_PORT             GPIOA
#define HAL_DEBUG_PIN1_UP()             HAL_GPIO_WritePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1, GPIO_PIN_SET)
#define HAL_DEBUG_PIN1_DOWN()           HAL_GPIO_WritePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN1_TOGGLE()         HAL_GPIO_TogglePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1)

#define HAL_DEBUG_PIN2_CLK_ENABLE()     __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN2                  GPIO_PIN_6
#define HAL_DEBUG_PIN2_PORT             GPIOA
#define HAL_DEBUG_PIN2_UP()             HAL_GPIO_WritePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2, GPIO_PIN_SET)
#define HAL_DEBUG_PIN2_DOWN()           HAL_GPIO_WritePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN2_TOGGLE()         HAL_GPIO_TogglePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2)

#define HAL_ASSERT_PIN_CLK_ENABLE()     __GPIOF_CLK_ENABLE()
#define HAL_ASSERT_PIN                  GPIO_PIN_8
#define HAL_ASSERT_PIN_PORT             GPIOF
#define HAL_ASSERT_PIN_UP()             HAL_GPIO_WritePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN, GPIO_PIN_SET)
#define HAL_ASSERT_PIN_DOWN()           HAL_GPIO_WritePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN, GPIO_PIN_RESET)
#define HAL_ASSERT_PIN_TOGGLE()         HAL_GPIO_TogglePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN)

// Joseph Yiu's method
// From http://forums.arm.com/index.php?showtopic=13949
#define    DWT_CYCCNT                   *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL                  *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR                    *(volatile uint32_t*)0xE000EDFC
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
///         D_LOG(D_DEBUG, "%d(ms)", CYCLES_TO_MS(cycles_diff));
#define CYCLES_TO_MS(cycles)            ((U32)((cycles) / SystemCoreClockKHz))

#define HAL_IO                          volatile

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

#endif // _HAL_OLIMEX_STM32_P407_H_
