/***************************************************************************//**
* @file    os_trigger.h
* @brief   OS Trigger.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TRIGGER_H_
#define _OS_TRIGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osal.h"
#include "os_timer.h"

/**
* \defgroup OS_Trigger OS_Trigger
* @{
*/
//------------------------------------------------------------------------------
typedef void* OS_TriggerHd;

enum {
    OS_TRIGGER_STATE_UNDEF,
    OS_TRIGGER_STATE_LAST
};
typedef U8 OS_TriggerState;

typedef OS_StorageItem OS_TriggerItem;

typedef struct {
    const OS_TimerConfig*   timer_cfg_p;
    OS_TriggerItem*         item_p;
    OS_TriggerState         state;
} OS_TriggerConfig;

typedef struct {
    OS_TriggerItem*         item_p;
    OS_TriggerState         state;
    OS_TimerStats           timer_stats;
} OS_TriggerStats;

//------------------------------------------------------------------------------
/// @brief      Create an trigger.
/// @param[in]  cfg_p           Trigger config.
/// @param[out] trigger_hd_p    Trigger handle.
/// @return     #Status.
Status          OS_TriggerCreate(const OS_TriggerConfig* cfg_p, OS_TriggerHd* trigger_hd_p);

/// @brief      Delete the trigger.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[in]  timeout         Operation timeout (ticks).
/// @return     #Status.
Status          OS_TriggerDelete(const OS_TriggerHd trigger_hd, const OS_TimeMs timeout);

/// @brief      Get the trigger timer.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[out] timer_hd_p      Timer handle.
/// @return     #Status.
Status          OS_TriggerTimerGet(const OS_TriggerHd trigger_hd, OS_TimerHd* timer_hd_p);

/// @brief      Get the trigger state.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[out] state_p         State.
/// @return     #Status.
Status          OS_TriggerStateGet(const OS_TriggerHd trigger_hd, OS_TriggerState* state_p);

/// @brief      Get the trigger state.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[out] period_p        Period.
/// @return     #Status.
Status          OS_TriggerPeriodGet(const OS_TriggerHd trigger_hd, OS_TimeMs* period_p);

/// @brief      Set the trigger state.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[in]  new_period      Trigger new period.
/// @param[in]  new_state       Trigger new state.
/// @param[in]  timeout         Operation timeout.
/// @return     #Status.
Status          OS_TriggerStatePeriodSet(const OS_TriggerHd trigger_hd, const OS_TimeMs new_period, const OS_TriggerState new_state,
                                         const OS_TimeMs timeout);

/// @brief      Create an trigger item.
/// @param[in]  data_p          Item's data.
/// @param[in]  size            Items's data size.
/// @param[out] item_pp         Item.
/// @return     #Status.
Status          OS_TriggerItemCreate(const void* data_p, const U16 size, OS_TriggerItem** item_pp);

/// @brief      Delete the trigger item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_TriggerItemDelete(OS_TriggerItem* item_p);

/// @brief      Add trigger item owner.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_TriggerItemOwnerAdd(OS_TriggerItem* item_p);

/// @brief      Lock trigger item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_TriggerItemLock(OS_TriggerItem* item_p, const OS_TimeMs timeout);

/// @brief      Unlock trigger item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_TriggerItemUnlock(OS_TriggerItem* item_p);

/// @brief      Get the trigger item.
/// @param[in]  trigger_hd      Trigger handle.
/// @return     Item.
OS_TriggerItem* OS_TriggerItemGet(const OS_TriggerHd trigger_hd);

/// @brief      Get the trigger item by timer id.
/// @param[in]  timer_id        Timer id.
/// @return     Item.
OS_TriggerItem* OS_TriggerItemByTimerIdGet(const OS_TimerId timer_id);

/// @brief      Get the trigger item by state.
/// @param[in]  state           State.
/// @return     Item.
OS_TriggerItem* OS_TriggerItemByStateGet(const OS_TriggerState state);

/// @brief      Get trigger statistics.
/// @param[in]  trigger_hd      Trigger handle.
/// @param[out] stats_p         Trigger statistics.
/// @return     #Status.
Status          OS_TriggerStatsGet(const OS_TriggerHd trigger_hd, OS_TriggerStats* stats_p);

/// @brief      Get the next trigger.
/// @param[in]  trigger_hd      Trigger handle.
/// @return     Trigger handle.
OS_TriggerHd    OS_TriggerNextGet(const OS_TriggerHd trigger_hd);

/**
* \addtogroup OS_ISR_Trigger ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------

/**@}*/ //OS_ISR_Trigger

/**@}*/ //OS_Trigger

#ifdef __cplusplus
}
#endif

#endif // _OS_TRIGGER_H_