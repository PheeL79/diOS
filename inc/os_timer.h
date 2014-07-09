/***************************************************************************//**
* @file    os_timer.h
* @brief   OS Timer.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "timers.h"
#include "os_time.h"
#include "os_signal.h"
#include "os_queue.h"

/**
* \defgroup OS_Timer OS_Timer
* @{
*/
//------------------------------------------------------------------------------
typedef TimerHandle_t   OS_TimerHd;
typedef OS_SignalData   OS_TimerId;

enum {
    OS_TIM_ID_UNDEF,
    OS_TIM_ID_APP           = 0x01,
    OS_TIM_ID_LAST
};

typedef enum {
    OS_TIM_OPT_UNDEF,
    OS_TIM_OPT_PERIODIC,
    OS_TIM_OPT_EVENT,
    //OS_TIM_OPT_PRIO_HIGH
} OS_TimerOptions;

typedef struct {
    ConstStrPtr     name_p;
    OS_QueueHd      slot;
    TimeMs          period;
    OS_TimerId      id;
    OS_TimerOptions options;
} OS_TimerConfig, OS_TimerStats;

//------------------------------------------------------------------------------
/// @brief      Create a timer.
/// @param[in]  cfg_p           Timer config.
/// @param[out] timer_hd_p      Timer handle.
/// @return     #Status.
Status          OS_TimerCreate(const OS_TimerConfig* cfg_p, OS_TimerHd* timer_hd_p);

/// @brief      Delete the timer.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TimerDelete(const OS_TimerHd timer_hd, const TimeMs timeout);

/// @brief      Reset the timer.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TimerReset(const OS_TimerHd timer_hd, const TimeMs timeout);

/// @brief      Start the timer.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TimerStart(const OS_TimerHd timer_hd, const TimeMs timeout);

/// @brief      Stop the timer.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TimerStop(const OS_TimerHd timer_hd, const TimeMs timeout);

/// @brief      Get the timer period.
/// @param[in]  timer_hd        Timer handle.
/// @param[out] period_p        Timer period.
/// @return     #Status.
Status          OS_TimerPeriodGet(const OS_TimerHd timer_hd, TimeMs* period_p);

/// @brief      Set the timer period.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  new_period      Timer new period.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TimerPeriodSet(const OS_TimerHd timer_hd, const TimeMs new_period, const TimeMs timeout);

/// @brief      Get the timer slot.
/// @param[in]  timer_hd        Timer handle.
/// @param[out] period_p        Timer slot.
/// @return     #Status.
//Status          OS_TimerSlotGet(const OS_TimerHd timer_hd, OS_QueueHd* slot_p);

/// @brief      Set the timer slot.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  new_slot        Timer new slot.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
//Status          OS_TimerSlotSet(const OS_TimerHd timer_hd, const OS_QueueHd new_slot, const TimeMs timeout);

/// @brief      Check the timer is active.
/// @param[in]  timer_hd        Timer handle.
/// @return     Bool.
BL              OS_TimerIsActive(const OS_TimerHd timer_hd);

/// @brief      Get the timer's id.
/// @param[in]  timer_hd        Timer handle.
/// @return     Timer id.
OS_TimerId      OS_TimerIdGet(const OS_TimerHd timer_hd);

/// @brief      Get the timer by id.
/// @param[in]  timer_id        Timer id.
/// @return     Timer handle.
OS_TimerHd      OS_TimerByIdGet(const OS_TimerId timer_id);

/// @brief      Get timer name.
/// @param[in]  timer_hd        Timer handle.
/// @return     Name.
ConstStrPtr     OS_TimerNameGet(const OS_TimerHd timer_hd);

/// @brief      Get the timer by its name.
/// @param[in]  name_p          Timer's name.
/// @return     Timer handle.
OS_TimerHd      OS_TimerByNameGet(ConstStrPtr name_p);

/// @brief      Get timer statistics.
/// @param[in]  timer_hd        Timer handle.
/// @param[out] stats_p         Queue statistics.
/// @return     #Status.
Status          OS_TimerStatsGet(const OS_TimerHd timer_hd, OS_TimerStats* stats_p);

/// @brief      Get the next timer.
/// @param[in]  timer_hd        Timer handle.
/// @return     Timer handle.
OS_TimerHd      OS_TimerNextGet(const OS_TimerHd timer_hd);

/**
* \addtogroup OS_ISR_Timer ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Reset the timer.
/// @param[in]  timer_hd        Timer handle.
/// @return     #Status.
Status          OS_ISR_TimerReset(const OS_TimerHd timer_hd);

/// @brief      Start the timer.
/// @param[in]  timer_hd        Timer handle.
/// @return     #Status.
Status          OS_ISR_TimerStart(const OS_TimerHd timer_hd);

/// @brief      Stop the timer.
/// @param[in]  timer_hd        Timer handle.
/// @return     #Status.
Status          OS_ISR_TimerStop(const OS_TimerHd timer_hd);

/// @brief      Change the timer period.
/// @param[in]  timer_hd        Timer handle.
/// @param[in]  new_period      Timer new period.
/// @return     #Status.
Status          OS_ISR_TimerPeriodChange(const OS_TimerHd timer_hd, const TimeMs new_period);

/**@}*/ //OS_ISR_Timer

/**@}*/ //OS_Timer

#ifdef __cplusplus
}
#endif

#endif // _OS_TIMER_H_
