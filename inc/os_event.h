/***************************************************************************//**
* @file    os_event.h
* @brief   OS Event.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_EVENT_H_
#define _OS_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osal.h"
#include "os_timer.h"

/**
* \defgroup OS_Event OS_Event
* @{
*/
//------------------------------------------------------------------------------
typedef void* OS_EventHd;

enum {
    OS_EVENT_STATE_UNDEF,
    OS_EVENT_STATE_LAST
};
typedef U8 OS_EventState;

typedef OS_StorageItem OS_EventItem;

typedef struct {
    const OS_TimerConfig*   timer_cfg_p;
    OS_EventItem*           item_p;
    OS_EventState           state;
} OS_EventConfig;

typedef struct {
    OS_EventItem*   item_p;
    OS_EventState   state;
    OS_TimerStats   timer_stats;
} OS_EventStats;

//------------------------------------------------------------------------------
/// @brief      Create an event.
/// @param[in]  cfg_p           Event config.
/// @param[out] ehd_p           Event handle.
/// @return     #Status.
Status          OS_EventCreate(const OS_EventConfig* cfg_p, OS_EventHd* ehd_p);

/// @brief      Delete the event.
/// @param[in]  ehd             Event handle.
/// @param[in]  timeout         Operation timeout (ticks).
/// @return     #Status.
Status          OS_EventDelete(const OS_EventHd ehd, const OS_TimeMs timeout);

/// @brief      Get the event timer.
/// @param[in]  ehd             Event handle.
/// @param[out] timer_hd_p      Timer handle.
/// @return     #Status.
Status          OS_EventTimerGet(const OS_EventHd ehd, OS_TimerHd* timer_hd_p);

/// @brief      Get the event state.
/// @param[in]  ehd             Event handle.
/// @param[out] state_p         State.
/// @return     #Status.
Status          OS_EventStateGet(const OS_EventHd ehd, OS_EventState* state_p);

/// @brief      Get the event state.
/// @param[in]  ehd             Event handle.
/// @param[out] period_p        Period.
/// @return     #Status.
Status          OS_EventPeriodGet(const OS_EventHd ehd, OS_TimeMs* period_p);

/// @brief      Set the event state.
/// @param[in]  ehd             Event handle.
/// @param[in]  new_period      Event new period.
/// @param[in]  new_state       Event new state.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_EventStatePeriodSet(const OS_EventHd ehd, const OS_TimeMs new_period, const OS_EventState new_state,
                                       const OS_TimeMs timeout);

/// @brief      Create an event item.
/// @param[in]  data_p          Item's data.
/// @param[in]  size            Items's data size.
/// @param[out] item_pp         Item.
/// @return     #Status.
Status          OS_EventItemCreate(const void* data_p, const U16 size, OS_EventItem** item_pp);

/// @brief      Delete the event item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_EventItemDelete(OS_EventItem* item_p);

/// @brief      Add event item owner.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_EventItemOwnerAdd(OS_EventItem* item_p);

/// @brief      Lock event item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_EventItemLock(OS_EventItem* item_p, const OS_TimeMs timeout);

/// @brief      Unlock event item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_EventItemUnlock(OS_EventItem* item_p);

/// @brief      Get the event item.
/// @param[in]  ehd             Event handle.
/// @return     Item.
OS_EventItem*   OS_EventItemGet(const OS_EventHd ehd);

/// @brief      Get the event item by timer id.
/// @param[in]  timer_id        Timer id.
/// @return     Item.
OS_EventItem*   OS_EventItemByTimerIdGet(const OS_TimerId timer_id);

/// @brief      Get the event item by state.
/// @param[in]  state           State.
/// @return     Item.
OS_EventItem*   OS_EventItemByStateGet(const OS_EventState state);

/// @brief      Get event statistics.
/// @param[in]  ehd             Event handle.
/// @param[out] stats_p         Event statistics.
/// @return     #Status.
Status          OS_EventStatsGet(const OS_EventHd ehd, OS_EventStats* stats_p);

/// @brief      Get the next event.
/// @param[in]  ehd             Event handle.
/// @return     Event handle.
OS_EventHd      OS_EventNextGet(const OS_EventHd ehd);

/**
* \addtogroup OS_ISR_Event ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------

/**@}*/ //OS_ISR_Event

/**@}*/ //OS_Event

#ifdef __cplusplus
}
#endif

#endif // _OS_EVENT_H_