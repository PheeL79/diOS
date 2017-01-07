/***************************************************************************//**
* @file    os_time.c
* @brief   OS Time.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "osal.h"
#include "os_environment.h"
#include "os_supervise.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_time.h"

//------------------------------------------------------------------------------
static Bool OS_DateYearIsLeap(const U16 year);
static Bool OS_DateIsWeekDay(const U8 week_day);

//------------------------------------------------------------------------------
extern volatile OS_Env os_env;
static OS_MutexHd os_time_mutex;

/******************************************************************************/
Status OS_TimeInit(void);
Status OS_TimeInit(void)
{
Status s = S_OK;
    os_time_mutex = OS_MutexCreate();
    if (OS_NULL == os_time_mutex) { return S_INVALID_PTR; }
    return s;
}

/******************************************************************************/
Status OS_TimeGet(const OS_TimeFormat format, OS_DateTime* os_time_p)
{
Status s = S_UNDEF;
    if (OS_NULL == os_time_p) { return S_INVALID_PTR; }
    IF_STATUS(s = OS_DriverIoCtl(os_env.drv_rtc, DRV_REQ_RTC_TIME_GET, (void*)os_time_p)) { return s; }
    //Convert time to specified format.
    switch (format) {
        case OS_TIME_UNDEF:
        case OS_TIME_UTC:
        case OS_TIME_GMT:
        case OS_TIME_GMT_OFFSET:
        case OS_TIME_LOCAL:
        case OS_TIME_UPTIME:
            break;
        default:
            OS_Trace(L_WARNING, "\nUndefined time format!");
            break;
    }
    return s;
}

/******************************************************************************/
Status OS_TimeSet(const OS_TimeFormat format, OS_DateTime* os_time_p)
{
Status s = S_UNDEF;
    if (OS_NULL == os_time_p) { return S_INVALID_PTR; }
    if (OS_TRUE != OS_TimeIsValid(os_time_p->hours, os_time_p->minutes, os_time_p->seconds)) {
        return S_INVALID_VALUE;
    }
    IF_STATUS(s = OS_DriverIoCtl(os_env.drv_rtc, DRV_REQ_RTC_TIME_SET, (void*)os_time_p)) { return s; }
    return s;
}

/******************************************************************************/
Bool OS_TimeIsValid(const U8 hours, const U8 minutes, const U8 seconds)
{
    //WARNING!!! Currently only for 24H mode!
    if (24 < hours)     { return OS_FALSE; }
    if (60 < minutes)   { return OS_FALSE; }
    if (60 < seconds)   { return OS_FALSE; }
    return OS_TRUE;
}

/******************************************************************************/
Status OS_DateGet(const OS_DateFormat format, OS_DateTime* os_date_p)
{
Status s = S_UNDEF;
    if (OS_NULL == os_date_p) { return S_INVALID_PTR; }
    IF_STATUS(s = OS_DriverIoCtl(os_env.drv_rtc, DRV_REQ_RTC_DATE_GET, (void*)os_date_p)) { return s; }
    return s;
}

/******************************************************************************/
Status OS_DateSet(const OS_DateFormat format, OS_DateTime* os_date_p)
{
Status s = S_OK;
    if (OS_NULL == os_date_p) { return S_INVALID_PTR; }
    if (OS_TRUE != OS_DateIsValid(os_date_p->year, os_date_p->month, os_date_p->day)) {
        return S_INVALID_VALUE;
    }
    os_date_p->weekday = OS_DateWeekDayGet(os_date_p->year, os_date_p->month, os_date_p->day);
    if (OS_TRUE != OS_DateIsWeekDay(os_date_p->weekday)) { return S_INVALID_VALUE; }
    IF_STATUS(s = OS_DriverIoCtl(os_env.drv_rtc, DRV_REQ_RTC_DATE_SET, (void*)os_date_p)) { return s; }
    return s;
}

/******************************************************************************/
//Code was taken from "musl" - an implementation of the standard library for Linux-based systems
Status OS_DateTimeUtcConvert(const OS_TimeS utc_timestamp_s, OS_DateTime* os_date_time_p)
{
#define DAYS_PER_400Y   ((365 * 400) + 97)
#define DAYS_PER_100Y   ((365 * 100) + 24)
#define DAYS_PER_4Y     ((365 * 4)   + 1)

#define SECS_IN_DAY     86400
#define DAYS_IN_YEAR    365

static const U8 days_in_month[] = { 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29 };
LongLong days, secs;
Int remdays, remsecs, remyears;
Int qc_cycles, c_cycles, q_cycles;
LongLong years;
Int months;
Int yday, leap;
Status s = S_OK;
    if (OS_NULL == os_date_time_p) { return S_INVALID_PTR; }
	/* Reject time_t values whose year would overflow int */
	secs = utc_timestamp_s;
	if ((secs < (INT_MIN * 31622400LL)) || (secs > (INT_MAX * 31622400LL)))
		return S_INVALID_VALUE;

	days = secs / SECS_IN_DAY;
	remsecs = secs % SECS_IN_DAY;
	if (remsecs < 0) {
		remsecs += SECS_IN_DAY;
		days--;
	}

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / DAYS_IN_YEAR;
	if (remyears == 4) remyears--;
	remdays -= remyears * DAYS_IN_YEAR;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= (DAYS_IN_YEAR + leap)) yday -= (DAYS_IN_YEAR + leap);

	years = remyears + (4 * q_cycles) + (100 * c_cycles) + (400 * qc_cycles);

	for (months = 0; days_in_month[months] <= remdays; months++) {
		remdays -= days_in_month[months];
    }

	if (((years + 100) > INT_MAX) || ((years + 100) < INT_MIN)) {
		return S_INVALID_VALUE;
    }

	os_date_time_p->year  = years + HAL_RTC_YEAR_BASE;
	os_date_time_p->month = months + 1;
	if (os_date_time_p->month >= 12) {
		os_date_time_p->month -= 12;
		os_date_time_p->year++;
	}
	os_date_time_p->day     = remdays;
	os_date_time_p->weekday = OS_DateWeekDayGet(os_date_time_p->year, os_date_time_p->month, os_date_time_p->day);

	os_date_time_p->hours   = remsecs / 3600;
	os_date_time_p->minutes = remsecs / 60 % 60;
	os_date_time_p->seconds = remsecs % 60;

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
Bool OS_DateIsValid(const U16 year, const U8 month, const U8 day)
{
const U8 days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    //STM32 RTC limitation.
    if ((HAL_RTC_YEAR_BASE > year) || (2099 < year)) {
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
Bool OS_DateYearIsLeap(const U16 year)
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
Size JND =                                                         \
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
Bool OS_DateIsWeekDay(const U8 week_day)
{
    if ((week_day < OS_WEEK_DAY_MONDAY) ||
        (week_day > OS_WEEK_DAY_SUNDAY)) {
        return OS_FALSE;
    }
    return OS_TRUE;
}

/******************************************************************************/
ConstStrP OS_TimeNameDayOfWeekGet(const OS_TimeWeekDay week_day, const Locale locale)
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
OS_Tick OS_TickCountGet(void)
{
    return xTaskGetTickCount();
}

/******************************************************************************/
OS_DateTime OS_TimeStringParse(ConstStrP time_p)
{
OS_DateTime time;
    OS_MemSet(&time, 0, sizeof(time));
    if (OS_NULL != time_p) {
        // Parse time string.
        char const delim_str[] = OS_LOCALE_TIME_DELIM_EN\
                                 OS_LOCALE_TIME_DELIM_RU;
        char* time_str_p = (char*)time_p;
        char* time_item_p = OS_StrToK(time_str_p, delim_str);
        Long time_val[3];

        for (Size i = 0; OS_NULL != time_item_p; ++i) {
            //TODO(A. Filyanov) Check for valid digits.
            time_val[i] = OS_StrToL((const char*)time_item_p, OS_NULL, 10);
            time_item_p = OS_StrToK(OS_NULL, delim_str);
        }
        time.hours      = (U8)time_val[0];
        time.minutes    = (U8)time_val[1];
        time.seconds    = (U8)time_val[2];
        time.daylight   = OS_TIME_DAYLIGHT_NONE;        //TODO(A. Filyanov) Parse a value!
        time.hourformat = OS_TIME_HOUR_FORMAT_UNDEF;    //TODO(A. Filyanov) Parse a value!
    }
    return time;
}

/******************************************************************************/
OS_DateTime OS_DateStringParse(ConstStrP date_p)
{
const Locale locale = OS_LocaleGet();
OS_DateTime date;
    OS_MemSet(&date, 0, sizeof(date));
    if (OS_NULL != date_p) {
        // Parse date string.
        char const delim_str[] = OS_LOCALE_DATE_DELIM_EN\
                                 OS_LOCALE_DATE_DELIM_RU;
        char* date_str_p = (char*)date_p;
        char* date_item_p = OS_StrToK(date_str_p, delim_str);
        Long date_val[3];

        for (Size i = 0; OS_NULL != date_item_p; ++i) {
            date_val[i] = OS_StrToL((const char*)date_item_p, OS_NULL, 10);
            date_item_p = OS_StrToK(OS_NULL, delim_str);
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