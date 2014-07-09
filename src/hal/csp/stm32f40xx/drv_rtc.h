/**************************************************************************//**
* @file    drv_rtc.h
* @brief   RTC driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_RTC_H_
#define _DRV_RTC_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_RTC,
    DRV_ID_RTC_LAST
};

enum {
    DRV_REQ_RTC_UNDEF = DRV_REQ_STD_LAST,
    DRV_REQ_RTC_BKUP_REG_READ,
    DRV_REQ_RTC_BKUP_REG_WRITE,
    DRV_REQ_RTC_BKUP_REGS_READ,
    DRV_REQ_RTC_BKUP_REGS_WRITE,
    DRV_REQ_RTC_ALARM_A_SET,
    DRV_REQ_RTC_ALARM_B_SET,
    DRV_REQ_RTC_TIME_GET,
    DRV_REQ_RTC_TIME_SET,
    DRV_REQ_RTC_DATE_GET,
    DRV_REQ_RTC_DATE_SET,
    DRV_REQ_RTC_LAST
};

typedef enum {
    HAL_RTC_BKUP_REG_IS_VALID,
    HAL_RTC_BKUP_REG_USER,
    HAL_RTC_BKUP_REG_LAST
} HAL_RTC_BackupReg;

typedef struct {
    const HAL_RTC_BackupReg reg;
    U32*                    val_p;
} HAL_RTC_BackupRegRead;

typedef struct {
    const HAL_RTC_BackupReg reg;
    const U32               val;
} HAL_RTC_BackupRegWrite;

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_rtc_v[];

#endif // _DRV_RTC_H_