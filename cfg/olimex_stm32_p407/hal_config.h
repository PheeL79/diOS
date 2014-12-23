/**************************************************************************//**
* @file    hal_config.h
* @brief   Config header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

#include "olimex_stm32_p407/hal_config.asm"

// HAL ------------------------------------------------------------------------
#define OLIMEX_STM32_P407
#define USE_HAL_DRIVER

// PWR
#define PWR_LEVEL                       PWR_PVDLevel_6

//RTC_CLOCK_SOURCE
//#define                                 RTC_CLOCK_SOURCE_LSI
#define                                 RTC_CLOCK_SOURCE_LSE
//STM32F4xx
#define RTC_BACKUP_REGS_MAX             20
#define RTC_YEAR_BASE                   2000U

// Buttons
#define BUTTON_WAKEUP_PIN               GPIO_PIN_0

// IWDG
#define TIMER_IWDG_ENABLED              0
#define TIMER_IWDG_TIMEOUT              3000

// DEBUG
#define HAL_ASSERT_LEVEL                D_DEBUG
#define HAL_LOG_LEVEL                   D_DEBUG
#define HAL_STDIO_TERM_HEIGHT           24
#define HAL_STDIO_TERM_WIDTH            80
#define HAL_STDIO_BUFF_LEN              512
#define drv_stdio_p                     drv_usart_v[DRV_ID_USART6]

// Timeouts
#define HAL_TIMEOUT_DRIVER              1000U
#define HAL_TIMEOUT_TIMER_TIMESTAMP     20

//SDIO SD
#define SDIO_SD_ENABLED                 1
#define SDIO_SDIO_DMA_STREAM3           3
//#define SDIO_SD_DMA_STREAM6             6
#define SDIO_SD_IRQ_PRIO                1
#define SDIO_SD_DMA_IRQ_PRIO            1
#define SD_CARD_BLOCK_SIZE              0x200
#define SD_CARD_SECTOR_SIZE             SD_CARD_BLOCK_SIZE

//USB Host
#define USBH_ENABLED                    1
#define USBH_FS_ENABLED                 1
#define USBH_HS_ENABLED                 0

//USB Host Classes
#define USBH_HID_ENABLED                0
#define USBH_MSC_ENABLED                1
#define USBH_MSC_BLOCK_SIZE             0x200
#define USBH_MSC_SECTOR_SIZE            USBH_MSC_BLOCK_SIZE

//USB Device
#define USBD_ENABLED                    0
#define USBD_FS_ENABLED                 0
#define USBD_HS_ENABLED                 1

//USB Device Classes
#define USBD_HID_ENABLED                0
#define USBD_MSC_ENABLED                1

// Memory
#define MEM_BLOCK_SIZE_MIN              8
#define MEM_INT_CCM_BASE_ADDRESS        0x10000000
#define MEM_INT_CCM_SIZE                0x10000
#define MEM_EXT_SRAM_BASE_ADDRESS       0x60000000
#define MEM_EXT_SRAM_SIZE               0x80000

//Enable External SRAM
#define DATA_IN_ExtSRAM

// APP ------------------------------------------------------------------------
// cstdlib redefenitions
// Memory
#define HAL_MemSet      memset
#define HAL_MemCmp      memcmp
#define HAL_MemCpy      memcpy
#define HAL_MemMov      memmove
// String
#define HAL_AtoI        atoi
#define HAL_StrLen      strlen
#define HAL_StrChr      strchr
#define HAL_StrCmp      strcmp
#define HAL_StrCpy      strcpy
#define HAL_StrCat      strcat
#define HAL_StrToK      strtok
#define HAL_StrToL      strtol
#define HAL_StrToUL     strtoul
#define HAL_StrNCpy     strncpy
#define HAL_SPrintF     sprintf
#define HAL_SNPrintF    snprintf

// Locale
#define LOCALE_STRING_EN                "en"
#define LOCALE_STRING_RU                "ru"
#define LOCALE_DEFAULT                  LOCALE_STRING_EN

#endif // _HAL_CONFIG_H_