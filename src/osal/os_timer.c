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
    OS_TimerOptions options;
} OS_TimerConfigDyn;

//------------------------------------------------------------------------------
static OS_List os_timers_list;
static OS_MutexHd os_timer_mutex;

//------------------------------------------------------------------------------
static void OS_TimerCallback(OS_TimerHd timer_hd);
static OS_TimerConfigDyn* OS_TimerConfigDynGet(const OS_TimerHd timer_hd);

/******************************************************************************/
#pragma inline
OS_TimerConfigDyn* OS_TimerConfigDynGet(const OS_TimerHd timer_hd)
{
const OS_ListItem* item_l_p = OS_ListItemByOwnerFind(&os_timers_list, (OS_Owner)timer_hd);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    return (OS_TimerConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
}

/******************************************************************************/
void OS_TimerCallback(OS_TimerHd timer_hd)
{
    const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
    if ((OS_NULL == timer_hd) || (OS_NULL == cfg_dyn_p)) {
        OS_LOG_S(D_WARNING, S_UNDEF_TIMER);
        return;
    }
    const OS_SignalId sig_id = BIT_TEST(cfg_dyn_p->options, OS_TIM_OPT_EVENT) ? OS_SIG_EVENT : OS_SIG_TIMER;
    const OS_Signal signal = OS_SIGNAL_CREATE(sig_id, OS_TimerIdGet(timer_hd));
    OS_SIGNAL_EMIT(cfg_dyn_p->slot, signal, OS_MSG_PRIO_HIGH);
}

/******************************************************************************/
Status OS_TimerInit(void);
Status OS_TimerInit(void)
{
    os_timer_mutex = OS_MutexCreate();
    if (OS_NULL == os_timer_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_timers_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_timers_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerCreate(const OS_TimerConfig* cfg_p, OS_TimerHd* timer_hd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
OS_TimerConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_TimerConfigDyn));
Status s = S_OK;

    if ((OS_NULL == item_l_p) || (OS_NULL == cfg_dyn_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL != OS_TimerByIdGet(cfg_p->id)) { s = S_INVALID_VALUE; goto error; } //TODO(A. Filyanov) Recursive mutex, move under lock!
    IF_STATUS_OK(s = OS_MutexLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        if (OS_NULL == cfg_p->slot) { s = S_UNDEF_QUEUE; goto error; }
        const OS_Tick period_ticks = OS_MS_TO_TICKS(cfg_p->period);
        *timer_hd_p = xTimerCreate(cfg_p->name_p, period_ticks, BIT_TEST(cfg_p->options, BIT(OS_TIM_OPT_PERIODIC)),
                                   (void*)cfg_p->id, OS_TimerCallback);
        if (OS_NULL == timer_hd_p) { s = S_UNDEF_TIMER; goto error; }
        cfg_dyn_p->name_p   = cfg_p->name_p;
        cfg_dyn_p->slot     = cfg_p->slot;
        cfg_dyn_p->period   = cfg_p->period;
        cfg_dyn_p->options  = cfg_p->options;
        OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)cfg_dyn_p);
        OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)*timer_hd_p);
        OS_ListAppend(&os_timers_list, item_l_p);
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    } OS_MutexUnlock(os_timer_mutex);
    return s;
}

/******************************************************************************/
Status OS_TimerDelete(const OS_TimerHd timer_hd, const TimeMs timeout)
{
OS_ListItem* item_l_p;
OS_TimerConfigDyn* cfg_dyn_p;
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
Status s = S_OK;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerDelete(timer_hd, timeout_ticks)) { return S_TIMEOUT; }
    IF_STATUS_OK(s = OS_MutexLock(os_timer_mutex, timeout)) {    // os_list protection;
        item_l_p = OS_ListItemByOwnerFind(&os_timers_list, (OS_Owner)timer_hd);
        if (OS_NULL == item_l_p) { s = S_INVALID_REF; goto error; }
        cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
        if (OS_NULL == cfg_dyn_p) { s = S_INVALID_REF; goto error; }
        OS_ListItemDelete(item_l_p);
        OS_Free(cfg_dyn_p);
    }
error:
    OS_MutexUnlock(os_timer_mutex);
    return s;
}

/******************************************************************************/
Status OS_TimerReset(const OS_TimerHd timer_hd, const TimeMs timeout)
{
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerReset(timer_hd, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerStart(const OS_TimerHd timer_hd, const TimeMs timeout)
{
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerStart(timer_hd, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerStop(const OS_TimerHd timer_hd, const TimeMs timeout)
{
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerStop(timer_hd, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_TimerPeriodGet(const OS_TimerHd timer_hd, TimeMs* period_p)
{
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    *period_p = ((OS_TimerConfigDyn*)timer_hd)->period;
    return S_OK;
}

/******************************************************************************/
Status OS_TimerPeriodSet(const OS_TimerHd timer_hd, const TimeMs new_period, const TimeMs timeout)
{
const OS_Tick timeout_ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
const OS_Tick new_period_ticks = ((OS_BLOCK == new_period) || (OS_NO_BLOCK == new_period)) ? new_period : OS_MS_TO_TICKS(new_period);
    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerChangePeriod(timer_hd, new_period_ticks, timeout_ticks)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
BL OS_TimerIsActive(const OS_TimerHd timer_hd)
{
    if (pdTRUE != xTimerIsTimerActive(timer_hd)) {
        return OS_FALSE;
    }
    return OS_TRUE;
}

/******************************************************************************/
OS_TimerId OS_TimerIdGet(const OS_TimerHd timer_hd)
{
    return (OS_TimerId)pvTimerGetTimerID(timer_hd);
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
        return ((OS_TimerConfigDyn*)timer_hd)->name_p;
    }
    return OS_NULL;
}

/******************************************************************************/
OS_TimerHd OS_TimerByNameGet(ConstStrPtr name_p)
{
OS_TimerHd timer_hd = OS_NULL;

    IF_STATUS_OK(OS_MutexLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        OS_ListItem* iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_timers_list);
        OS_TimerConfig* timer_cfg_p;

        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) {
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            timer_cfg_p = (OS_TimerConfig*)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            if (!strcmp((const char*)name_p, (const char*)timer_cfg_p->name_p)) {
                timer_hd = (OS_TimerHd)OS_LIST_ITEM_OWNER_GET(iter_li_p);
                break;
            }
        }
    } OS_MutexUnlock(os_timer_mutex);
    return timer_hd;
}

/******************************************************************************/
Status OS_TimerStatsGet(const OS_TimerHd timer_hd, OS_TimerStats* stats_p)
{
Status s = S_OK;
    if (OS_NULL != timer_hd) {
        IF_STATUS_OK(s = OS_MutexLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
            const OS_TimerConfigDyn* cfg_dyn_p = OS_TimerConfigDynGet(timer_hd);
            if (OS_NULL != cfg_dyn_p) {
                stats_p->name_p     = cfg_dyn_p->name_p;
                stats_p->slot       = OS_QueueParentGet(cfg_dyn_p->slot);
                stats_p->id         = OS_TimerIdGet(timer_hd);
                stats_p->period     = cfg_dyn_p->period;
                stats_p->options    = cfg_dyn_p->options;
            } else { s = S_INVALID_REF; }
        } OS_MutexUnlock(os_timer_mutex);
    } else { s = S_UNDEF_TIMER; }
    return s;
}

/******************************************************************************/
OS_TimerHd OS_TimerNextGet(const OS_TimerHd timer_hd)
{
OS_TimerHd tim_hd = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_timer_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* iter_li_p;
        if (OS_NULL == timer_hd) {
            iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_timers_list);
            if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                tim_hd = (OS_TimerHd)OS_LIST_ITEM_OWNER_GET(iter_li_p);
            }
        } else {
            iter_li_p = OS_ListItemByOwnerFind(&os_timers_list, (OS_Owner)timer_hd);
            if (OS_NULL != iter_li_p) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) { goto error; }
                tim_hd = (OS_TimerHd)OS_LIST_ITEM_OWNER_GET(iter_li_p);
            }
        }
    }
error:
    OS_MutexUnlock(os_timer_mutex);
    return tim_hd;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_TimerReset(const OS_TimerHd timer_hd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == timer_hd) { return S_UNDEF_TIMER; }
    if (pdTRUE != xTimerResetFromISR(timer_hd, &xHigherPriorityTaskWoken)) {
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
    if (pdTRUE != xTimerStartFromISR(timer_hd, &xHigherPriorityTaskWoken)) {
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
    if (pdTRUE != xTimerStopFromISR(timer_hd, &xHigherPriorityTaskWoken)) {
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
    if (pdTRUE != xTimerChangePeriodFromISR(timer_hd, new_period_ticks, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}