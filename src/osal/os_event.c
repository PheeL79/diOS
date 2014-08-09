/***************************************************************************//**
* @file    os_event.c
* @brief   OS Event.
* @author  A. Filyanov
*******************************************************************************/
#include "os_common.h"
#include "os_memory.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_list.h"
#include "os_event.h"

//------------------------------------------------------------------------------
typedef struct {
    OS_TimerHd      timer_hd;
    OS_EventItem*   item_p;
    OS_EventState   state;
} OS_EventConfigDyn;
//OS_EventConfigDyn* == OS_EventHd;

//------------------------------------------------------------------------------
static OS_List os_events_list;
static OS_MutexHd os_event_mutex;
static volatile U32 events_count = 0;

/******************************************************************************/
#pragma inline
static OS_EventConfigDyn* OS_EventConfigDynGet(const OS_EventHd ehd);
OS_EventConfigDyn* OS_EventConfigDynGet(const OS_EventHd ehd)
{
const OS_ListItem* item_l_p = (OS_ListItem*)ehd;
OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
    return cfg_dyn_p;
}

/******************************************************************************/
Status OS_EventInit(void);
Status OS_EventInit(void)
{
    os_event_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_event_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_events_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_events_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_EventCreate(const OS_EventConfig* cfg_p, OS_EventHd* ehd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
const OS_EventHd ehd = (OS_EventHd)item_l_p;
OS_EventConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_EventConfigDyn));
Status s = S_OK;

    if ((OS_NULL == item_l_p) || (OS_NULL == cfg_dyn_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL == ehd_p) { s = S_INVALID_REF;  goto error; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_TimerHd timer_hd;
        IF_STATUS_OK(s = OS_TimerCreate(cfg_p->timer_cfg_p, &timer_hd)) {
            cfg_dyn_p->item_p   = cfg_p->item_p;
            cfg_dyn_p->state    = cfg_p->state;
            cfg_dyn_p->timer_hd = timer_hd;
            OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)cfg_dyn_p);
            OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)cfg_p->timer_cfg_p->id);
            OS_ListAppend(&os_events_list, item_l_p);
            ++events_count;
            if (OS_NULL != ehd_p) {
                *ehd_p = ehd;
            }
            if (0 < cfg_p->timer_cfg_p->period) {
                IF_STATUS(s = OS_TimerStart(timer_hd, OS_TIMEOUT_DEFAULT)) {
                }
            }
        }
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    }
    return s;
}

/******************************************************************************/
Status OS_EventDelete(const OS_EventHd ehd, const TimeMs timeout)
{
OS_ListItem* item_l_p = (OS_ListItem*)ehd;
Status s = S_OK;

    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, timeout)) {  // os_list protection;
        OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerDelete(timer_hd, timeout)) {
            goto error;
        }
        s = OS_EventItemDelete(cfg_dyn_p->item_p);
        if (S_INVALID_REF == s) { //ignore NULL storage
            s = S_OK;
        }
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        --events_count;
error:
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EventTimerGet(const OS_EventHd ehd, OS_TimerHd* timer_hd_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        if (OS_NULL != timer_hd_p) {
            OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
            *timer_hd_p = cfg_dyn_p->timer_hd;
        }
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EventStateGet(const OS_EventHd ehd, OS_EventState* state_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        *state_p = cfg_dyn_p->state;
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EventPeriodGet(const OS_EventHd ehd, TimeMs* period_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerPeriodGet(timer_hd, period_p)) {
            OS_LOG_S(D_WARNING, s);
        }
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EventStatePeriodSet(const OS_EventHd ehd, const TimeMs new_period, const OS_EventState new_state,
                              const TimeMs timeout)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, timeout)) {  // os_list protection;
        OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        cfg_dyn_p->state = new_state;
        IF_STATUS(s = OS_TimerPeriodSet(timer_hd, new_period, timeout)) {
            OS_LOG_S(D_WARNING, s);
        }
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EventItemCreate(const void* data_p, const U16 size, OS_EventItem** item_pp)
{
    return OS_StorageItemCreate(data_p, size, (OS_StorageItem**)item_pp);
}

/******************************************************************************/
Status OS_EventItemDelete(OS_EventItem* item_p)
{
    return OS_StorageItemDelete((OS_StorageItem*)item_p);
}

/******************************************************************************/
Status OS_EventItemOwnerAdd(OS_EventItem* item_p)
{
    return OS_StorageItemOwnerAdd((OS_StorageItem*)item_p);
}

/******************************************************************************/
Status OS_EventItemLock(OS_StorageItem* item_p, const TimeMs timeout)
{
    return OS_StorageItemLock((OS_StorageItem*)item_p, timeout);
}

/******************************************************************************/
Status OS_EventItemUnlock(OS_StorageItem* item_p)
{
    return OS_StorageItemUnlock((OS_StorageItem*)item_p);
}

/******************************************************************************/
OS_EventItem* OS_EventItemGet(const OS_EventHd ehd)
{
OS_EventItem* item_p = OS_NULL;
    if (OS_NULL == ehd) { return OS_NULL; }
    IF_STATUS_OK(OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        item_p = cfg_dyn_p->item_p;
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return item_p;
}

/******************************************************************************/
OS_EventItem* OS_EventItemByTimerIdGet(const OS_TimerId timer_id)
{
OS_EventItem* item_p = OS_NULL;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_ListItem* item_l_p = OS_ListItemByOwnerGet(&os_events_list, (OS_Owner)timer_id);
        if (OS_NULL != item_l_p) {
            const OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
            item_p = cfg_dyn_p->item_p;
        }
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return item_p;
}

/******************************************************************************/
OS_EventItem* OS_EventItemByStateGet(const OS_EventState state)
{
OS_EventHd ehd = OS_NULL;
    while (OS_NULL != (ehd = OS_EventNextGet(ehd))) {
        const OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        if (state == cfg_dyn_p->state) {
            return cfg_dyn_p->item_p;
        }
    }
    return OS_NULL;
}

/******************************************************************************/
Status OS_EventStatsGet(const OS_EventHd ehd, OS_EventStats* stats_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_EventConfigDyn* cfg_dyn_p = OS_EventConfigDynGet(ehd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerStatsGet(timer_hd, &stats_p->timer_stats)) { goto error; }
        stats_p->item_p     = cfg_dyn_p->item_p;
        stats_p->state      = cfg_dyn_p->state;
error:
        OS_MutexRecursiveUnlock(os_event_mutex);
    }
    return s;
}

/******************************************************************************/
OS_EventHd OS_EventNextGet(const OS_EventHd ehd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)ehd;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_events_list));
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
        OS_MutexRecursiveUnlock(os_event_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_EventHd)iter_li_p;
}