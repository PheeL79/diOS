/**************************************************************************//**
* @file    hal_config.h
* @brief   Config header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

#include "olimex_stm32_p407/hal_config.asm"

// HAL ------------------------------------------------------------------------
#define STM32F40XX
#define OLIMEX_STM32_P407
#define USE_STDPERIPH_DRIVER

// PWR
#define PWR_LEVEL                       PWR_PVDLevel_6

//RTC_CLOCK_SOURCE
#define                                 RTC_LSI
//#define                                 RTC_LSE
//STM32F4xx
#define RTC_BACKUP_REGS_MAX             20
#define RTC_YEAR_BASE                   2000U

// IWDT
//#define USE_IWDT
#define HAL_WATCHDOG_TIMEOUT            3000
#define HAL_WATCHDOG_RESET()            IWDG_ReloadCounter()

// DEBUG
#define D_LOG_LEVEL_DEFAULT             D_DEBUG
#define D_STDIO_TERM_HEIGHT             24
#define D_STDIO_TERM_WIDTH              80
#define D_STDIO_BUFF_LEN                1024
#define drv_stdio_p                     drv_usart_v[DRV_ID_USART6]

// TIMER
#define CFG_TIMER_MILESTONE_TIMEOUT     20

//SD CARD
#define SD_SDIO_ENABLED                 1
#define SD_SDIO_DMA_STREAM3             3
//#define SD_SDIO_DMA_STREAM6             6
#define SD_SDIO_IRQ_PRIO                1
#define SD_SDIO_DMA_IRQ_PRIO            1
#define SD_SDIO_BLOCK_SIZE              512
#define SD_CARD_SECTOR_SIZE             SD_SDIO_BLOCK_SIZE

// Memory
#define MEM_BLOCK_SIZE_MIN              8
#define MEM_INT_CCM_BASE_ADDRESS        0x10000000
#define MEM_INT_CCM_SIZE                0x10000
#define MEM_EXT_SRAM_BASE_ADDRESS       0x60000000
#define MEM_EXT_SRAM_SIZE               0x80000

//Enable External SRAM
#define DATA_IN_ExtSRAM

// APP ------------------------------------------------------------------------
#define LOCALE_STRING_EN                "en"
#define LOCALE_STRING_RU                "ru"
#define LOCALE_DEFAULT                  LOCALE_STRING_EN

#endif // _HAL_CONFIG_H_