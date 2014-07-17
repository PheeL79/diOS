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
    OS_EventItem*   item_p;
    OS_EventState   state;
    OS_TimerId      timer_id;
} OS_EventConfigDyn;
//OS_EventConfigDyn* == OS_EventHd;

//------------------------------------------------------------------------------
static OS_List os_events_list;
static OS_MutexHd os_event_mutex;
static volatile U32 events_count = 0;

/******************************************************************************/
Status OS_EventInit(void);
Status OS_EventInit(void)
{
    os_event_mutex = OS_MutexCreate();
    if (OS_NULL == os_event_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_events_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_events_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_EventCreate(const OS_EventConfig* cfg_p, OS_EventHd* ehd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
OS_EventConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_EventConfigDyn));
Status s = S_OK;

    if ((OS_NULL == item_l_p) || (OS_NULL == cfg_dyn_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL == ehd_p) { s = S_INVALID_REF;  goto error; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_events_list protection;
        OS_TimerHd timer_hd;
        IF_STATUS_OK(s = OS_TimerCreate(cfg_p->timer_cfg_p, &timer_hd)) {
            *ehd_p              = (OS_EventHd)cfg_dyn_p;
            cfg_dyn_p->item_p   = cfg_p->item_p;
            cfg_dyn_p->state    = cfg_p->state;
            cfg_dyn_p->timer_id = cfg_p->timer_cfg_p->id;
            OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)cfg_dyn_p);
            OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)cfg_dyn_p->timer_id);
            OS_ListAppend(&os_events_list, item_l_p);
            ++events_count;
            if (0 < cfg_p->timer_cfg_p->period) {
                IF_STATUS(s = OS_TimerStart(timer_hd, OS_TIMEOUT_DEFAULT)) {
                }
            }
        }
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    } OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
Status OS_EventDelete(const OS_EventHd ehd, const TimeMs timeout)
{
OS_ListItem* item_l_p;
OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
Status s = S_OK;

    if ((OS_NULL == ehd) || (OS_NULL == cfg_dyn_p)) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, timeout)) {  // os_events_list protection;
        item_l_p = OS_ListItemByValueFind(&os_events_list, (OS_Value)ehd);
        if (OS_NULL == item_l_p) { s = S_INVALID_REF; goto error; }
        const OS_TimerHd timer_hd = OS_TimerByIdGet(cfg_dyn_p->timer_id);
        if (OS_NULL == timer_hd) { s = S_UNDEF_TIMER; goto error; }
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
    }
error:
    OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
Status OS_EventTimerGet(const OS_EventHd ehd, OS_TimerHd* timer_hd_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_events_list protection;
        OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        *timer_hd_p = OS_TimerByIdGet(cfg_dyn_p->timer_id);
    } OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
Status OS_EventStateGet(const OS_EventHd ehd, OS_EventState* state_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_events_list protection;
        OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        *state_p = cfg_dyn_p->state;
    } OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
Status OS_EventPeriodGet(const OS_EventHd ehd, TimeMs* period_p)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_events_list protection;
        OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        const OS_TimerHd timer_hd = OS_TimerByIdGet(cfg_dyn_p->timer_id);
        if (OS_NULL == timer_hd) { s = S_UNDEF_TIMER; goto error; }
        IF_STATUS(s = OS_TimerPeriodGet(timer_hd, period_p)) {
            OS_LOG_S(D_WARNING, s);
        }
    }
error:
    OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
Status OS_EventStatePeriodSet(const OS_EventHd ehd, const TimeMs new_period, const OS_EventState new_state,
                              const TimeMs timeout)
{
Status s = S_OK;
    if (OS_NULL == ehd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, timeout)) {  // os_events_list protection;
        OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        cfg_dyn_p->state = new_state;
        const OS_TimerHd timer_hd = OS_TimerByIdGet(cfg_dyn_p->timer_id);
        if (OS_NULL == timer_hd) { s = S_UNDEF_TIMER; goto error; }
        IF_STATUS(s = OS_TimerPeriodSet(timer_hd, new_period, timeout)) {
            OS_LOG_S(D_WARNING, s);
        }
    }
error:
    OS_MutexUnlock(os_event_mutex);
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
    IF_STATUS_OK(OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_events_list protection;
        OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        item_p = cfg_dyn_p->item_p;
    } OS_MutexUnlock(os_event_mutex);
    return item_p;
}

/******************************************************************************/
OS_EventItem* OS_EventItemByTimerIdGet(const OS_TimerId timer_id)
{
OS_EventItem* item_p = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_events_list protection;
        const OS_ListItem* item_l_p = OS_ListItemByOwnerFind(&os_events_list, (OS_Owner)timer_id);
        if (OS_NULL != item_l_p) {
            const OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
            if ((OS_NULL != cfg_dyn_p) || (OS_DELAY_MAX != (OS_Value)cfg_dyn_p)) {
                item_p = cfg_dyn_p->item_p;
            }
        }
    } OS_MutexUnlock(os_event_mutex);
    return item_p;
}

/******************************************************************************/
OS_EventItem* OS_EventItemByStateGet(const OS_EventState state)
{
OS_EventHd ehd = OS_NULL;
    while (OS_NULL != (ehd = OS_EventNextGet(ehd))) {
        const OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
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
    IF_STATUS_OK(s = OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_EventConfigDyn* cfg_dyn_p = (OS_EventConfigDyn*)ehd;
        if (OS_NULL == cfg_dyn_p) { s = S_INVALID_REF; goto error; }
        const OS_TimerHd timer_hd = OS_TimerByIdGet(cfg_dyn_p->timer_id);
        if (OS_NULL == timer_hd) { s = S_INVALID_REF; goto error; }
        IF_STATUS(s = OS_TimerStatsGet(timer_hd, &stats_p->timer_stats)) { goto error; }
        stats_p->item_p     = cfg_dyn_p->item_p;
        stats_p->state      = cfg_dyn_p->state;
    }
error:
    OS_MutexUnlock(os_event_mutex);
    return s;
}

/******************************************************************************/
OS_EventHd OS_EventNextGet(const OS_EventHd ehd)
{
OS_ListItem* iter_li_p;
OS_EventHd event_hd = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_event_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        if (OS_NULL == ehd) {
            iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_events_list);
            if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) { goto error; }
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            event_hd = (OS_EventHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
        } else {
            iter_li_p = OS_ListItemByValueFind(&os_events_list, (OS_Value)ehd);
            if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) { goto error; }
                event_hd = (OS_EventHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            }
        }
    }
error:
    OS_MutexUnlock(os_event_mutex);
    return event_hd;
}