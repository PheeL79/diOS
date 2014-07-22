/***************************************************************************//**
* @file    os_timer.c
* @brief   OS Timer.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_common.h"
#include "os_memory.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_list.h"
#include "os_message.h"
#include "os_timer.h"

//------------------------------------------------------------------------------
typedef TimerCallbackFunction_t OS_TimerCallbackFunc;
typedef struct {
    ConstStrPtr     name_p;
    OS_QueueHd      slot;
    TimeMs          period;
    OS_TimerId      id;
    OS_TimerOptions options;
} OS_TimerConfigDyn;

//------------------------------------------------------------------------------
static OS_List os_timers_list;
static OS_MutexHd os_timer_mutex;

//------------------------------------------------------------------------------
static void OS_TimerCallback(const TimerHandle_t timer_handle);

/******************************************************************************/
#pragma inline
static OS_TimerConfigDyn* OS_TimerConfigDynGet(const OS_TimerHd timer_hd);
OS_TimerConfigDyn* OS_TimerConfigDynGet(const OS_TimerHd timer_hd)
{
const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
OS_TimerConfigDyn* cfg_dyn_p = (OS_TimerConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
    return cfg_dyn_p;
}

/******************************************************************************/
void OS_TimerCallback(const TimerHandle_t timer_handle)
{
    const OS_TimerConfigDyn* cfg_dyn_p = (OS_TimerConfigDyn*)pvTimerGetTimerID(timer_handle);
    if ((OS_NULL == timer_handle) || (OS_NULL == cfg_dyn_p)) {
        OS_LOG_S(D_WARNING, S_UNDEF_TIMER);
        return;
    }
    const OS_SignalId sig_id = BIT_TEST(cfg_dyn_p->options, OS_TIM_OPT_EVENT) ? OS_SIG_EVENT : OS_SIG_TIMER;
    const OS_Signal signal = OS_SIGNAL_CREATE(sig_id, cfg_dyn_p->id);
    OS_SIGNAL_EMIT(cfg_dyn_p->slot, signal, OS_MSG_PRIO_HIGH);
}

/******************************************************************************/
Status OS_TimerInit(void);
Status OS_TimerInit(void)
{
    os_timer_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_timer_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_timers_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_timers_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerCreate(const OS_TimerConfig* cfg_p, OS_TimerHd* timer_hd_p)
{
Status s = S_OK;

    if (OS_NULL == cfg_p) { return S_INVALID_REF; }
    if (OS_NULL == cfg_p->slot) { return S_UNDEF_QUEUE; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    const OS_TimerHd timer_hd = (OS_TimerHd)item_l_p;
    OS_TimerConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_TimerConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        if (OS_NULL == OS_TimerByIdGet(cfg_p->id)) {
            const OS_Tick period_ticks = OS_MS_TO_TICKS(cfg_p->period);
            const TimerHandle_t timer_handle = xTimerCreate(cfg_p->name_p, period_ticks,
                                                            BIT_TEST(cfg_p->options, BIT(OS_TIM_OPT_PERIODIC)),
                                                            (void*)cfg_dyn_p, OS_TimerCallback);
            if (OS_NULL == timer_handle) { s = S_UNDEF_TIMER; goto error; }
            cfg_dyn_p->name_p   = cfg_p->name_p;
            cfg_dyn_p->slot     = cfg_p->slot;
            cfg_dyn_p->period   = cfg_p->period;
            cfg_dyn_p->id       = cfg_p->id;
            cfg_dyn_p->options  = cfg_p->options;
            OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)cfg_dyn_p);
            OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)timer_handle);
            OS_ListAppend(&os_timers_list, item_l_p);
            if (OS_NULL != timer_hd_p) {
                *timer_hd_p = timer_hd;
            }
        } else { s = S_INVALID_VALUE; }
error:
        IF_STATUS(s) {
            OS_Free(cfg_dyn_p);
            OS_ListItemDelete(item_l_p);
        }
        OS_MutexRecursiveUnlock(os_timer_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TimerDelete(const OS_TimerHd timer_hd, const TimeMs timeout)
{
OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
Status s = S_OK;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerDelete(timer_handle, timeout_ticks)) { return S_TIMEOUT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_timer_mutex, timeout)) {    // os_list protection;
        OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
        OS_ListItemDelete(item_l_p);
        OS_Free(cfg_dyn_p);
        OS_MutexRecursiveUnlock(os_timer_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TimerReset(const OS_TimerHd timer_hd, const TimeMs timeout)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (pdTRUE != xTimerReset(timer_handle, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerStart(const OS_TimerHd timer_hd, const TimeMs timeout)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (pdTRUE != xTimerStart(timer_handle, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerStop(const OS_TimerHd timer_hd, const TimeMs timeout)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (pdTRUE != xTimerStop(timer_handle, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerPeriodGet(const OS_TimerHd timer_hd, TimeMs* period_p)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
    if (OS_NULL == cfg_dyn_p) { return S_INVALID_REF; }
    *period_p = cfg_dyn_p->period;
    return S_OK;
}

/******************************************************************************/
Status OS_TimerPeriodSet(const OS_TimerHd timer_hd, const TimeMs new_period, const TimeMs timeout)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    const OS_Tick new_period_ticks = ((OS_BLOCK == new_period) || (OS_NO_BLOCK == new_period)) ? new_period : OS_MS_TO_TICKS(new_period);
    if (pdTRUE != xTimerChangePeriod(timer_handle, new_period_ticks, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
BL OS_TimerIsActive(const OS_TimerHd timer_hd)
{
    if (OS_NULL == timer_hd) { return OS_FALSE; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    if (pdTRUE != xTimerIsTimerActive(timer_handle)) {
        return OS_FALSE;
    }
    return OS_TRUE;
}

/******************************************************************************/
OS_TimerId OS_TimerIdGet(const OS_TimerHd timer_hd)
{
    if (OS_NULL == timer_hd) { return OS_SIGNAL_DATA_MASK; }
    const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
    return cfg_dyn_p->id;
}

/******************************************************************************/
OS_TimerHd OS_TimerByIdGet(const OS_TimerId timer_id)
{
OS_TimerHd timer_hd = OS_NULL;
    while (OS_NULL != (timer_hd = OS_TimerNextGet(timer_hd))) {
        if (timer_id == OS_TimerIdGet(timer_hd)) {
            break;
        }
    }
    return timer_hd;
}

/******************************************************************************/
ConstStrPtr OS_TimerNameGet(const OS_TimerHd timer_hd)
{
    if (OS_NULL != timer_hd) {
        const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
        if (OS_NULL != cfg_dyn_p) {
            return cfg_dyn_p->name_p;
        }
    }
    return OS_NULL;
}

/******************************************************************************/
OS_TimerHd OS_TimerByNameGet(ConstStrPtr name_p)
{
OS_TimerHd timer_hd = OS_NULL;

    IF_STATUS_OK(OS_MutexRecursiveLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        OS_ListItem* iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_timers_list));

        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) {
            const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
            if (!strcmp((const char*)name_p, (const char*)cfg_dyn_p->name_p)) {
                timer_hd = (OS_TimerHd)iter_li_p;
                break;
            }
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
        }
    } OS_MutexRecursiveUnlock(os_timer_mutex);
    return timer_hd;
}

/******************************************************************************/
Status OS_TimerStatsGet(const OS_TimerHd timer_hd, OS_TimerStats* stats_p)
{
Status s = S_OK;
    if (OS_NULL != timer_hd) {
        IF_STATUS_OK(s = OS_MutexRecursiveLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
            const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
            if (OS_NULL != cfg_dyn_p) {
                stats_p->name_p     = cfg_dyn_p->name_p;
                stats_p->slot       = OS_QueueParentGet(cfg_dyn_p->slot);
                stats_p->id         = OS_TimerIdGet(timer_hd);
                stats_p->period     = cfg_dyn_p->period;
                stats_p->options    = cfg_dyn_p->options;
            } else { s = S_INVALID_REF; }
            OS_MutexRecursiveUnlock(os_timer_mutex);
        }
    } else { s = S_UNDEF_TIMER; }
    return s;
}

/******************************************************************************/
OS_TimerHd OS_TimerNextGet(const OS_TimerHd timer_hd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)timer_hd;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_timers_list));
            if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                iter_li_p = OS_NULL;
            }
        } else {
            if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                    iter_li_p = OS_NULL;
                }
            } else {
                iter_li_p = OS_NULL;
            }
        }
        OS_MutexRecursiveUnlock(os_timer_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_TimerHd)iter_li_p;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_TimerReset(const OS_TimerHd timer_hd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    if (pdTRUE != xTimerResetFromISR(timer_handle, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ISR_TimerStart(const OS_TimerHd timer_hd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    if (pdTRUE != xTimerStartFromISR(timer_handle, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ISR_TimerStop(const OS_TimerHd timer_hd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    if (pdTRUE != xTimerStopFromISR(timer_handle, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ISR_TimerPeriodChange(const OS_TimerHd timer_hd, const TimeMs new_period)
{
const OS_Tick new_period_ticks = ((OS_BLOCK == new_period) || (OS_NO_BLOCK == new_period)) ? new_period : OS_MS_TO_TICKS(new_period);
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    const OS_ListItem* item_l_p = (OS_ListItem*)timer_hd;
    const TimerHandle_t timer_handle = (TimerHandle_t)OS_LIST_ITEM_OWNER_GET(item_l_p);
    if (pdTRUE != xTimerChangePeriodFromISR(timer_handle, new_period_ticks, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}