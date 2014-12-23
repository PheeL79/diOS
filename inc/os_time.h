/***************************************************************************//**
* @file    os_time.h
* @brief   OS Time.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TIME_H_
#define _OS_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_common.h"
#include "typedefs.h"

/**
* \defgroup OS_Time OS_Time
* @{
*/
//------------------------------------------------------------------------------
typedef enum {
    OS_WEEK_DAY_UNDEF,
    OS_WEEK_DAY_MONDAY,
    OS_WEEK_DAY_TUESDAY,
    OS_WEEK_DAY_WEDNESDAY,
    OS_WEEK_DAY_THURSDAY,
    OS_WEEK_DAY_FRIDAY,
    OS_WEEK_DAY_SATURDAY,
    OS_WEEK_DAY_SUNDAY,
    OS_WEEK_DAY_LAST
} OS_TimeWeekDay;

typedef enum {
    OS_TIME_UNDEF,
    OS_TIME_GMT,
    OS_TIME_GMT_OFFSET,
    OS_TIME_LOCAL,
    OS_TIME_UPTIME,
    OS_TIME_LAST
} OS_TimeFormat;

typedef enum {
    OS_DATE_UNDEF,
    OS_DATE_LAST
} OS_DateFormat;

typedef enum {
    OS_ALARM_UNDEF,
    OS_ALARM_LAST
} OS_AlarmFormat;

typedef enum {
    OS_TIME_DAYLIGHT_UNDEF,
    OS_TIME_DAYLIGHT_NONE,
    OS_TIME_DAYLIGHT_SUMMER,
    OS_TIME_DAYLIGHT_WINTER,
    OS_TIME_DAYLIGHT_LAST
} OS_TimeDayLight;

typedef enum {
    OS_TIME_HOUR_FORMAT_UNDEF,
    OS_TIME_HOUR_FORMAT_AM,
    OS_TIME_HOUR_FORMAT_PM,
    OS_TIME_HOUR_FORMAT_LAST
} OS_TimeHourFormat;

typedef Time            OS_DateTime;
//typedef RTC_AlarmTypeDef OS_Alarm;
typedef TickType_t      OS_Tick;
typedef U32             TimeMs;
typedef U32             TimeS;

/// Converts from RTOS ticks to milliseconds.
#define OS_TICKS_TO_MS(ticks)       ((U32)(((ticks) * KHZ) / configTICK_RATE_HZ))

/// Converts from milliseconds to RTOS ticks, value is always > 0.
#pragma inline
static inline U32 OS_MS_TO_TICKS(const TimeMs ms)
{
    return ((ms * configTICK_RATE_HZ) / KHZ);
}

//------------------------------------------------------------------------------
/// @brief      Get the current time.
/// @param[in]  format          Time format.
/// @param[out] os_time_p       Time data.
/// @return     #Status.
Status          OS_TimeGet(const OS_TimeFormat format, OS_DateTime* os_time_p);

/// @brief      Set time.
/// @param[in]  format          Time format.
/// @param[in]  os_time_p       Time data.
/// @return     #Status.
Status          OS_TimeSet(const OS_TimeFormat format, OS_DateTime* os_time_p);

/// @brief      Get the current date.
/// @param[in]  format          Date format.
/// @param[out] os_date_p       Date data.
/// @return     #Status.
Status          OS_DateGet(const OS_DateFormat format, OS_DateTime* os_date_p);

/// @brief      Set date.
/// @param[in]  format          Date format.
/// @param[in]  os_date_p       Date data.
/// @return     #Status.
Status          OS_DateSet(const OS_DateFormat format, OS_DateTime* os_date_p);

//Status          OS_AlarmGet(const OS_AlarmFormat format, /*const OS_AlarmEvent event,*/ OS_Alarm* os_alarm_p);
//Status        OS_AlarmSet(const OS_AlarmFormat format, /*const OS_AlarmEvent event,*/ OS_Alarm* os_alarm_p);

/// @brief      Time validation.
/// @param[in]  hours           Hours.
/// @param[in]  minutes         Minutes.
/// @param[in]  seconds         Seconds.
/// @return     Bool.
/// @warning    Currently only for 24H mode!
Bool            OS_TimeIsValid(const U8 hours, const U8 minutes, const U8 seconds);

/// @brief      Date validation.
/// @param[in]  year            Year.
/// @param[in]  month           Month.
/// @param[in]  day             Day.
/// @return     Bool.
Bool            OS_DateIsValid(const U16 year, const U8 month, const U8 day);

/// @brief      Get the day of the week.
/// @param[in]  year            Year.
/// @param[in]  month           Month.
/// @param[in]  day             Day.
/// @return     Day of the week.
OS_TimeWeekDay  OS_DateWeekDayGet(const U16 year, const U8 month, const U8 day);

/// @brief      Get the day of the week name.
/// @param[in]  week_day        Day of week.
/// @param[in]  locale          Locale.
/// @return     Day of the week string.
ConstStrP       OS_TimeNameDayOfWeekGet(const OS_TimeWeekDay week_day, const Locale locale);

/// @brief      Get tick count.
/// @return     Tick count.
/// @note       Get the current tick count since system start.
OS_Tick         OS_TickCountGet(void);

/// @brief      Parse the time string.
/// @param[in]  time_p          String of time.
/// @return     Time.
/// @note       String delimiter format depends on the current locale settings.
/// @details    Example string for the EN locale: HH:MM:SS
OS_DateTime     OS_TimeStringParse(ConstStrP time_p);

/// @brief      Parse the date string.
/// @param[in]  date_p           String of date.
/// @return     Date.
/// @note       String delimiter format depends on the current locale settings.
/// @details    Example string for the EN locale: MM/DD/YYYY
OS_DateTime     OS_DateStringParse(ConstStrP date_p);

/**
* \addtogroup OS_ISR_Time ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Get tick count.
/// @return     Tick count.
/// @note       Get the current tick count since system uptime.
OS_Tick         OS_ISR_TickCountGet(void);

/**@}*/ //OS_ISR_Time

/**@}*/ //OS_Time

#ifdef __cplusplus
}
#endif

#endif // _OS_TIME_H_
