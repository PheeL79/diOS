/**************************************************************************//**
* @file    hal_config.h
* @brief   Config header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

// HAL ------------------------------------------------------------------------

// PWR
#define HAL_PWR_LEVEL                                   PWR_PVDLevel_6

//RTC_CLOCK_SOURCE
//#define                                             RTC_CLOCK_SOURCE_LSI
#define                                             RTC_CLOCK_SOURCE_LSE
//STM32F4xx
#define HAL_RTC_BACKUP_REGS_MAX                     20
#define HAL_RTC_YEAR_BASE                           2000U

// IWDG
#define HAL_TIMER_IWDG_ENABLED                      0
#define HAL_TIMER_IWDG_TIMEOUT                      3000

// DEBUG
#define HAL_ASSERT_LEVEL                            D_DEBUG
#define HAL_LOG_LEVEL                               D_DEBUG
#define HAL_STDIO_TERM_HEIGHT                       24
#define HAL_STDIO_TERM_WIDTH                        80
#define HAL_STDIO_BUFF_LEN                          512
#define drv_stdio_p                                 drv_usart_v[DRV_ID_USART6]
#define drv_stderr_p

// Timeouts
#define HAL_TIMEOUT_DRIVER                          1000U
#define HAL_TIMEOUT_TIMER_TIMESTAMP                 20

//CRC
#define HAL_CRC_ENABLED                             1

//SDIO SD
#define HAL_SDIO_SD_ENABLED                         1
#define HAL_SDIO_SDIO_DMA_STREAM3                   3
//#define HAL_SDIO_SD_DMA_STREAM6                     6
#define HAL_SD_CARD_BLOCK_SIZE                      0x200
#define HAL_SD_CARD_SECTOR_SIZE                     HAL_SD_CARD_BLOCK_SIZE

//USB Host
#define HAL_USBH_ENABLED                            0
#define HAL_USBH_FS_ENABLED                         1
#define HAL_USBH_HS_ENABLED                         0

//USB Host Classes
#define HAL_USBH_HID_ENABLED                        0
#define HAL_USBH_MSC_ENABLED                        1
#define HAL_USBH_MSC_BLOCK_SIZE                     0x200
#define HAL_USBH_MSC_SECTOR_SIZE                    USBH_MSC_BLOCK_SIZE

//USB Device
#define HAL_USBD_ENABLED                            1
#define HAL_USBD_FS_ENABLED                         0
#define HAL_USBD_HS_ENABLED                         1

//USB Device Classes
#define HAL_USBD_AUDIO_ENABLED                      0
#define HAL_USBD_HID_ENABLED                        0
#define HAL_USBD_MSC_ENABLED                        1

//Ethernet
#define HAL_ETH_ENABLED                             1
#define HAL_ETH_MAC_ADDR_SIZE                       6
#define HAL_ETH_MAC_ADDR0                           0x00
#define HAL_ETH_MAC_ADDR1                           0x80
#define HAL_ETH_MAC_ADDR2                           0xE1
#define HAL_ETH_MAC_ADDR3                           0x00
#define HAL_ETH_MAC_ADDR4                           0x00
#define HAL_ETH_MAC_ADDR5                           0x00
#define HAL_ETH_MTU_SIZE                            1500

// Memory
#define HAL_MEM_BLOCK_SIZE_MIN                      8
#define HAL_MEM_INT_CCM_BASE_ADDRESS                0x10000000
#define HAL_MEM_INT_CCM_SIZE                        0x10000
#define HAL_MEM_EXT_SRAM_BASE_ADDRESS               0x60000000
#define HAL_MEM_EXT_SRAM_SIZE                       0x80000

//Enable External SRAM
#define DATA_IN_ExtSRAM

// cstdlib redefenitions
// Memory
#define HAL_MemSet                                  memset
#define HAL_MemCmp                                  memcmp
#define HAL_MemCpy                                  memcpy
#define HAL_MemMov                                  memmove
// String
#define HAL_AtoI                                    atoi
#define HAL_StrLen                                  strlen
#define HAL_StrChr                                  strchr
#define HAL_StrCmp                                  strcmp
#define HAL_StrCpy                                  strcpy
#define HAL_StrCat                                  strcat
#define HAL_StrToK                                  strtok
#define HAL_StrToL                                  strtol
#define HAL_StrToUL                                 strtoul
#define HAL_StrNCpy                                 strncpy
#define HAL_SPrintF                                 sprintf
#define HAL_SNPrintF                                snprintf
#define HAL_SScanF                                  sscanf

// Locale
#define HAL_LOCALE_STRING_EN                        "en"
#define HAL_LOCALE_STRING_RU                        "ru"
#define HAL_LOCALE_DEFAULT                          HAL_LOCALE_STRING_EN

#include "hal_config.S"
#include "hal_config_prio.h"
#include "hal\bsp\olimex_stm32_p407\bsp_config.h"

#endif // _HAL_CONFIG_H_