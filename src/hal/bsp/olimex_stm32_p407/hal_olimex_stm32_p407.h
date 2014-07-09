/**************************************************************************//**
* @file    hal_olimex_stm32_p407.h
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_OLIMEX_STM32_P407_H_
#define _HAL_OLIMEX_STM32_P407_H_

#include "stm32f4xx.h"
#include "common.h"

//CSP
#include "drv_dma.h"
#include "drv_rtc.h"
#include "drv_gpio.h"
#include "drv_nvic.h"
#include "drv_sdio.h"
#include "drv_timer.h"
#include "drv_usart.h"
#include "drv_power.h"
//BSP
#include "drv_mem_ext.h"
#include "drv_button.h"
#include "drv_led.h"

//-----------------------------------------------------------------------------
#define CRITICAL_ENTER()                __disable_irq()
#define CRITICAL_EXIT()                 __enable_irq()

#define SYSTICK_START()                 { SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk; }
#define SYSTICK_STOP()                  { SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; }

//#define HAL_DEBUG_PIN                   GPIO_Pin_5
//#define HAL_DEBUG_PIN_PORT              GPIOC
//#define HAL_DEBUG_PIN_UP                GPIO_SetBits(HAL_DEBUG_PIN_PORT, HAL_DEBUG_PIN)
//#define HAL_DEBUG_PIN_DOWN              GPIO_ResetBits(HAL_DEBUG_PIN_PORT, HAL_DEBUG_PIN)
//#define HAL_DEBUG_PIN_TOGGLE            GPIO_ToggleBits(HAL_DEBUG_PIN_PORT, HAL_DEBUG_PIN)

#define HAL_ASSERT_PIN                  GPIO_Pin_8
#define HAL_ASSERT_PIN_PORT             GPIOF
#define HAL_ASSERT_PIN_UP               GPIO_SetBits(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN)
#define HAL_ASSERT_PIN_DOWN             GPIO_ResetBits(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN)
#define HAL_ASSERT_PIN_TOGGLE           GPIO_ToggleBits(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN)

// Joseph Yiu's method
// From http://forums.arm.com/index.php?showtopic=13949
#define    DWT_CYCCNT                   *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL                  *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR                    *(volatile uint32_t*)0xE000EDFC
extern int cycles_diff;
extern int cycles_last;

#define CORE_CYCLES                     DWT_CYCCNT
#define DWT_STOPWATCH_START             { cycles_last = CORE_CYCLES; }
#define DWT_STOPWATCH_STOP              { cycles_diff = CORE_CYCLES - cycles_last; }

extern U32 SystemCoreClockKHz;
///@brief
///@details Example:
///         DWT_STOPWATCH_START;
///         HAL_DelayMs(100);
///         DWT_STOPWATCH_STOP;
///         D_LOG(D_DEBUG, "%d(ms)", CYCLES_TO_MS(cycles_diff));
#define CYCLES_TO_MS(cycles)            ((U32)((cycles) / SystemCoreClockKHz))

typedef volatile U32 HAL_IO_Reg;

//-----------------------------------------------------------------------------
/// @brief      Init NVIC struct.
/// @return     None.
void            NVIC_StructInit(NVIC_InitTypeDef* nvic_init_struct_p);

//-----------------------------------------------------------------------------
/// @brief      Init HAL.
/// @return     #Status.
Status          HAL_Init(void);

/// @brief      Delay (us).
/// @param[in]  delay_us        Delay value (us).
/// @return     None.
void            HAL_DelayUs(U32 delay_us);

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
