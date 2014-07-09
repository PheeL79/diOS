/***************************************************************************//**
* @file    os_time.c
* @brief   OS Time.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "stm32f4xx_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "os_environment.h"
#include "os_supervise.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_time.h"

//------------------------------------------------------------------------------
static BL OS_DateYearIsLeap(const U16 year);
static BL OS_DateIsWeekDay(const U8 week_day);

//------------------------------------------------------------------------------
static OS_MutexHd os_time_mutex;

/******************************************************************************/
Status OS_TimeInit(void);
Status OS_TimeInit(void)
{
Status s = S_OK;
    os_time_mutex = OS_MutexCreate();
    if (OS_NULL == os_time_mutex) { return S_INVALID_REF; }
    return s;
}

/******************************************************************************/
Status OS_TimeGet(const OS_TimeFormat format, OS_DateTime* os_time_p)
{
RTC_TimeTypeDef time;
Status s;
    if (OS_NULL == os_time_p) { return S_INVALID_REF; }
    IF_STATUS_OK(s = OS_MutexLock(os_time_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        /* Get the current Time */
        RTC_GetTime(RTC_Format_BIN, &time);
    } OS_MutexUnlock(os_time_mutex);
    os_time_p->hour = time.RTC_Hours;
    os_time_p->min  = time.RTC_Minutes;
    os_time_p->sec  = time.RTC_Seconds;
    //Convert time to specified format.
    switch (format) {
        case OS_TIME_UNDEF:
        case OS_TIME_GMT:
        case OS_TIME_GMT_OFFSET:
        case OS_TIME_LOCAL:
        case OS_TIME_UPTIME:
            break;
        default:
            OS_Trace(D_WARNING, "\nUndefined time format!");
            break;
    }
    return s;
}

/******************************************************************************/
Status OS_TimeSet(const OS_TimeFormat format, OS_DateTime* os_time_p)
{
RTC_TimeTypeDef RTC_TimeStruct;
Status s;
    if (OS_NULL == os_time_p) { return S_INVALID_REF; }
    if (OS_TRUE != OS_TimeIsValid(os_time_p->hour, os_time_p->min, os_time_p->sec)) {
        return S_INVALID_VALUE;
    }
    RTC_TimeStructInit(&RTC_TimeStruct);
    RTC_TimeStruct.RTC_Hours    = os_time_p->hour;
    RTC_TimeStruct.RTC_Minutes  = os_time_p->min;
    RTC_TimeStruct.RTC_Seconds  = os_time_p->sec;
    IF_STATUS_OK(s = OS_MutexLock(os_time_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        if (ERROR == RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct)) { return S_HARDWARE_FAULT; }
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct); //update value in shadow register.
    } OS_MutexUnlock(os_time_mutex);
    return s;
}

/******************************************************************************/
BL OS_TimeIsValid(const U8 hour, const U8 min, const U8 sec)
{
    //WARNING!!! Currently only for 24H mode!
    if (24 < hour)  { return OS_FALSE; }
    if (60 < min)   { return OS_FALSE; }
    if (60 < sec)   { return OS_FALSE; }
    return OS_TRUE;
}

/******************************************************************************/
Status OS_DateGet(const OS_DateFormat format, OS_DateTime* os_date_p)
{
RTC_DateTypeDef date;
Status s;
    if (OS_NULL == os_date_p) { return S_INVALID_REF; }
    IF_STATUS_OK(s = OS_MutexLock(os_time_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        /* Get the current Date */
        RTC_GetDate(RTC_Format_BIN, &date);
    } OS_MutexUnlock(os_time_mutex);
    os_date_p->year     = date.RTC_Year + RTC_YEAR_BASE;
    os_date_p->month    = date.RTC_Month;
    os_date_p->wday     = date.RTC_WeekDay;
    os_date_p->day      = date.RTC_Date;
    return s;
}

/******************************************************************************/
Status OS_DateSet(const OS_DateFormat format, OS_DateTime* os_date_p)
{
RTC_DateTypeDef RTC_DateStruct;
Status s = S_OK;
    if (OS_NULL == os_date_p) { return S_INVALID_REF; }
    if (OS_TRUE != OS_DateIsValid(os_date_p->year, os_date_p->month, os_date_p->day)) {
        return S_INVALID_VALUE;
    }
    os_date_p->wday = OS_DateWeekDayGet(os_date_p->year, os_date_p->month, os_date_p->day);
    if (OS_TRUE != OS_DateIsWeekDay(os_date_p->wday)) { return S_INVALID_VALUE; }
    RTC_DateStructInit(&RTC_DateStruct);
    RTC_DateStruct.RTC_Year     = os_date_p->year - RTC_YEAR_BASE;
    RTC_DateStruct.RTC_Month    = os_date_p->month;
    RTC_DateStruct.RTC_WeekDay  = os_date_p->wday;
    RTC_DateStruct.RTC_Date     = os_date_p->day;
    IF_STATUS_OK(s = OS_MutexLock(os_time_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        if (ERROR == RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct)) {
            s = S_HARDWARE_FAULT;
            goto error;
        }
        RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct); //update value in shadow register.
    }
error:
    OS_MutexUnlock(os_time_mutex);
    return s;
}

/******************************************************************************/
//Status OS_AlarmGet(const OS_AlarmFormat format, /*const OS_AlarmEvent event,*/ OS_Alarm* os_alarm_p)
//{
//    OS_CriticalSectionEnter(); {
//        /* Get the current Alarm */
//        RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, os_alarm_p);
//    } OS_CriticalSectionExit();
//    return S_OK;
//}

/******************************************************************************/
//http://www.glenmccl.com/004.htm#tag04
BL OS_DateIsValid(const U16 year, const U8 month, const U8 day)
{
const U8 days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    //STM32 RTC limitation.
    if ((RTC_YEAR_BASE > year) || (2099 < year)) {
        return OS_FALSE;
    }
    if ((1 > month) || (12 < month)) {
        return OS_FALSE;
    }
    if (1 > day) {
        return OS_FALSE;
    }
    if ((day <= days_in_month[month - 1]) || ((2 == month) && (29 == day) && OS_DateYearIsLeap(year))) {
        return OS_TRUE;
    }
    return OS_FALSE;
}

/******************************************************************************/
// return 1 if year is a leap year, else 0
BL OS_DateYearIsLeap(const U16 year)
{
    if (year % 4) {
        return OS_FALSE;
    }
    if (year % 100) {
        return OS_TRUE;
    }
    if (year % 400) {
        return OS_FALSE;
    }
    return OS_TRUE;
}

/******************************************************************************/
//http://stackoverflow.com/questions/6054016/c-program-to-find-day-of-week-given-date/6057429#6057429
OS_TimeWeekDay OS_DateWeekDayGet(const U16 year, const U8 month, const U8 day)
{
SIZE JND =                                                         \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045;
  return (OS_TimeWeekDay)((JND % 7) + OS_WEEK_DAY_MONDAY);
}

/******************************************************************************/
BL OS_DateIsWeekDay(const U8 week_day)
{
    if ((week_day < OS_WEEK_DAY_MONDAY) ||
        (week_day > OS_WEEK_DAY_SUNDAY)) {
        return OS_FALSE;
    }
    return OS_TRUE;
}

/******************************************************************************/
ConstStrPtr OS_TimeNameDayOfWeekGet(const OS_TimeWeekDay week_day, const Locale locale)
{
const LocaleString week_days[] = {
    { "Monday",     "Понедельник"   },
    { "Tuesday",    "Вторник"       },
    { "Wednesday",  "Среда"         },
    { "Thursday",   "Четверг"       },
    { "Friday",     "Пятница"       },
    { "Saturday",   "Суббота"       },
    { "Sunday",     "Воскресенье"   },
};
    return week_days[week_day - 1].locale[locale];
}

/******************************************************************************/
OS_TimeDayLight OS_TimeDayLightSavingsGet(void)
{
    return (OS_TimeDayLight)(RTC_GetStoreOperation() + (U32)OS_TIME_DAYLIGHT_SUMMER);
}

/******************************************************************************/
Status OS_TimeDayLightSavingsSet(const OS_TimeDayLight savings)
{
U32 mode, bck_bit;
Status s;
    if (OS_TIME_DAYLIGHT_SUMMER == savings) {
        mode    = RTC_DayLightSaving_ADD1H;
        bck_bit = RTC_StoreOperation_Set;
    } else if (OS_TIME_DAYLIGHT_WINTER == savings) {
        mode    = RTC_DayLightSaving_SUB1H;
        bck_bit = RTC_StoreOperation_Reset;
    } else {
        return S_INVALID_VALUE;
    }
    IF_STATUS_OK(s = OS_MutexLock(os_time_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        RTC_DayLightSavingConfig(mode, bck_bit);
    } OS_MutexUnlock(os_time_mutex);
    return s;
}

/******************************************************************************/
OS_Tick OS_TickCountGet(void)
{
    return xTaskGetTickCount();
}

/******************************************************************************/
OS_DateTime OS_TimeStringParse(ConstStrPtr time_p)
{
OS_DateTime time;
    memset(&time, 0, sizeof(time));
    if (OS_NULL != time_p) {
        // Parse time string.
        char const delim_str[] = OS_LOCALE_TIME_DELIM_EN\
                                 OS_LOCALE_TIME_DELIM_RU;
        char* time_str_p = (char*)time_p;
        char* time_item_p = strtok(time_str_p, delim_str);
        LNG time_val[3];

        for (SIZE i = 0; OS_NULL != time_item_p; ++i) {
            //TODO(A. Filyanov) Check for valid digits.
            time_val[i] = strtol((const char*)time_item_p, OS_NULL, 10);
            time_item_p = strtok(OS_NULL, delim_str);
        }
        time.hour   = (U8)time_val[0];
        time.min    = (U8)time_val[1];
        time.sec    = (U8)time_val[2];
    }
    return time;
}

/******************************************************************************/
OS_DateTime OS_DateStringParse(ConstStrPtr date_p)
{
const Locale locale = OS_LocaleGet();
OS_DateTime date;
    memset(&date, 0, sizeof(date));
    if (OS_NULL != date_p) {
        // Parse date string.
        char const delim_str[] = OS_LOCALE_DATE_DELIM_EN\
                                 OS_LOCALE_DATE_DELIM_RU;
        char* date_str_p = (char*)date_p;
        char* date_item_p = strtok(date_str_p, delim_str);
        LNG date_val[3];

        for (SIZE i = 0; OS_NULL != date_item_p; ++i) {
            date_val[i] = strtol((const char*)date_item_p, OS_NULL, 10);
            date_item_p = strtok(OS_NULL, delim_str);
        }
        if (LOC_RU == locale) {
            date.day    = (U8)date_val[0];
            date.month  = (U8)date_val[1];
        } else {
            date.month  = (U8)date_val[0];
            date.day    = (U8)date_val[1];
        }
        date.year = (U16)date_val[2];
    }
    return date;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
OS_Tick OS_ISR_TickCountGet(void)
{
    return xTaskGetTickCountFromISR();
}