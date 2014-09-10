/**************************************************************************//**
* @file    os_config.h
* @brief   Config header file for OS.
* @author  A. Filyanov
******************************************************************************/
#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

//Ensure that is only used by the compiler, and not the assembler.
#ifdef __ICCARM__
#include "olimex_stm32_p407/hal_config.h"
#include "os_memory.h"

//------------------------------------------------------------------------------
#define OS_IS_PREEMPTIVE                1
#define OS_MPU_ENABLED                  0
#define OS_STATS_ENABLED                1
#define OS_DEBUG_ENABLED                1
#define OS_TASK_DEADLOCK_TEST_ENABLED   1

// Timeouts.
#define OS_TIMEOUT_DEFAULT          1000U
#define OS_TIMEOUT_POWER            5000U
#define OS_TIMEOUT_DRIVER           HAL_TIMEOUT_DRIVER
#define OS_TIMEOUT_MUTEX_LOCK       1000U
#define OS_TIMEOUT_FS               1000U

// Ticks.
#define OS_TICK_RATE                1000U
#define OS_PULSE_RATE               1000U
#define OS_IDLE_TICKLESS_ENABLED    0
#define OS_IDLE_TICKS_TO_SLEEP      1000U
#endif // __ICCARM__

#define OS_PRIORITY_MIN             0x1
#define OS_PRIORITY_MAX             0x8
#define OS_PRIORITY_INT_MIN         0xF
#define OS_PRIORITY_INT_MAX         OS_PRIORITY_MAX

#ifdef __ICCARM__
//Memory
/// @brief   Memory type.
enum {
    OS_MEM_RAM_INT_SRAM,
    OS_MEM_RAM_INT_CCM,
    OS_MEM_RAM_EXT_SRAM,
    OS_MEM_LAST,
    OS_MEM_UNDEF
};
#define OS_MEM_HEAP_SYS             OS_MEM_RAM_INT_SRAM
#define OS_MEM_HEAP_APP             OS_MEM_RAM_INT_SRAM
#define OS_STACK_SIZE_MIN           0x100
#define OS_HEAP_SIZE                0x19FFF //0x1BFFF

/*__no_init*/ static U8 heap_int_sram[OS_HEAP_SIZE];            //@ RAM_region;
/*__no_init*/ static U8 heap_int_ccm[MEM_INT_CCM_SIZE]          @ MEM_INT_CCM_BASE_ADDRESS;
__no_init static U8 heap_ext_sram[MEM_EXT_SRAM_SIZE]        @ MEM_EXT_SRAM_BASE_ADDRESS;

/// @brief   Memory config vector.
static const OS_MemoryDesc memory_cfg_v[] = {
    { (U32)&heap_int_sram,      sizeof(heap_int_sram),      MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_INT_SRAM,    "SRAM Int." },
    { (U32)&heap_int_ccm,       sizeof(heap_int_ccm),       MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_INT_CCM,     "CCM"       },
    { (U32)&heap_ext_sram,      sizeof(heap_ext_sram),      MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_EXT_SRAM,    "SRAM Ext." },
    { 0,                        0,                          0,                      OS_MEM_LAST,            ""          }
};

#define OS_TIMERS_ENABLED           1
#define OS_TIMERS_TASK_PRIO         2
#define OS_TIMERS_QUEUE_LEN         10
#define OS_TIMERS_STACK_SIZE        OS_STACK_SIZE_MIN * 2

// Names length
#define OS_DRIVER_NAME_LEN          9
#define OS_TASK_NAME_LEN            12
#define OS_TIMER_NAME_LEN           12

#define OS_LOG_LEVEL_DEFAULT        "debug"
#define OS_LOG_STRING_LEN           128
#define OS_LOG_TIMER_STEP           3000
#define OS_LOG_FILE_PATH            "1:/log.txt"

//File system
//Look in ffconf.h for details
#define OS_FILE_SYSTEM_ENABLED      1
#define OS_FILE_SYSTEM_MAKE_ENABLED 1
#define OS_FILE_SYSTEM_TINY         0
#define OS_FILE_SYSTEM_READONLY     0
#define OS_FILE_SYSTEM_MINIMIZE     0
#define OS_FILE_SYSTEM_STRFUNC_EN   1
#define OS_FILE_SYSTEM_FASTSEEK     0
#define OS_FILE_SYSTEM_LABEL_EN     1
#define OS_FILE_SYSTEM_FORWARD_EN   0
#define OS_FILE_SYSTEM_CODEPAGE     1251
#define OS_FILE_SYSTEM_STRF_ENCODE  3
#define OS_FILE_SYSTEM_REL_PATH     2
#define OS_FILE_SYSTEM_MULTI_PART   0
#define OS_FILE_SYSTEM_ERASE_SEC_EN 0
#define OS_FILE_SYSTEM_NO_INFO      0
#define OS_FILE_SYSTEM_WORD_ACCESS  0
#define OS_FILE_SYSTEM_LOCK         0
#define OS_FILE_SYSTEM_REENTRANT    1
#define OS_FILE_SYSTEM_VOLUMES_MAX  4
#define OS_FILE_SYSTEM_VOLUME_STR_LEN       4
#define OS_FILE_SYSTEM_VOLUME_NAME_LEN      12
#define OS_FILE_SYSTEM_LONG_NAMES_ENABLED   3
#define OS_FILE_SYSTEM_LONG_NAMES_LEN       255
#define OS_FILE_SYSTEM_LONG_NAMES_UNICODE   0
#define OS_FILE_SYSTEM_DRV_DELIM    ':'
#define OS_FILE_SYSTEM_DIR_DELIM    '/'
#define OS_FILE_SYSTEM_SYNC_OBJ     OS_MutexHd
#define OS_FILE_SYSTEM_YEAR_BASE    1980U

//Settings
#define OS_SETTINGS_BROWSE_ENABLED  1
#define OS_SETTINGS_BUFFER_LEN      256
#define OS_SETTINGS_VALUE_LEN       16
#define OS_SETTINGS_VALUE_DEFAULT   ""
#define OS_SETTINGS_FILE_PATH       "1:/config.ini"

//Shell
#define OS_SHELL_HEIGHT             HAL_STDIO_TERM_HEIGHT
#define OS_SHELL_WIDTH              HAL_STDIO_TERM_WIDTH
#define OS_SHELL_PROMPT_ROOT        #
#define OS_SHELL_PROMPT_USER        $
#define OS_SHELL_PROMPT             OS_SHELL_PROMPT_ROOT
#define OS_SHELL_CL_LEN             1024
#define OS_SHELL_CL_ARGS_MAX        7
//Disable to local terminal edit
#define OS_SHELL_EDIT_ENABLED       0
#define OS_SHELL_HELP_ENABLED       1

//Locale
#define OS_LOCALE_DATE_DELIM_EN     "/"
#define OS_LOCALE_DATE_DELIM_RU     "."
#define OS_LOCALE_TIME_DELIM_EN     ":"
#define OS_LOCALE_TIME_DELIM_RU     OS_LOCALE_TIME_DELIM_EN

//Tests
//#define OS_TEST

// cstdlib
#define OS_AtoI     HAL_AtoI
#define OS_StrLen   HAL_StrLen
#define OS_StrChr   HAL_StrChr
#define OS_StrCmp   HAL_StrCmp
#define OS_StrCpy   HAL_StrCpy
#define OS_StrCat   HAL_StrCat
#define OS_StrToL   HAL_StrToL
#define OS_StrToK   HAL_StrToK
#define OS_StrNCpy  HAL_StrNCpy

enum {
    OS_SHELL_BUTTON_UP              = 'A',
    OS_SHELL_BUTTON_DOWN            = 'B',
    OS_SHELL_BUTTON_RIGHT           = 'C',
    OS_SHELL_BUTTON_LEFT            = 'D',
    OS_SHELL_BUTTON_HOME            = '1',
    OS_SHELL_BUTTON_END             = '4',
    OS_SHELL_BUTTON_DELETE          = '3',

    OS_ASCII_EOL                    = '\0',
    OS_ASCII_SPACE                  = ' ',
    OS_ASCII_EXC_MARK               = '!',
    OS_ASCII_TILDE                  = '~',
    OS_ASCII_DOUBLE_QUOTE           = '\"',
    OS_ASCII_ESC_CR                 = '\r',
    OS_ASCII_ESC_LF                 = '\n',
    OS_ASCII_ESC_BACKSPACE          = 0x7F,
    OS_ASCII_ESC_NOT_ECHO_NEXT      = 0x1B,
    OS_ASCII_ESC_OP_SQUARE_BRAC     = 0x5B
};

#endif // __ICCARM__
#endif // _OS_CONFIG_H_