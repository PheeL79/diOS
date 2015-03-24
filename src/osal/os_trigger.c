/***************************************************************************//**
* @file    os_trigger.c
* @brief   OS Event.
* @author  A. Filyanov
*******************************************************************************/
#include "os_common.h"
#include "os_memory.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_list.h"
#include "os_trigger.h"

#if (OS_TRIGGERS_ENABLED)
//------------------------------------------------------------------------------
typedef struct {
    OS_TimerHd      timer_hd;
    OS_TriggerItem* item_p;
    OS_TriggerState state;
} OS_TriggerConfigDyn;
//OS_TriggerConfigDyn* == OS_TriggerHd;

//------------------------------------------------------------------------------
static OS_List os_triggers_list;
static OS_MutexHd os_trigger_mutex;
static U32 triggers_count = 0;

/******************************************************************************/
static OS_TriggerConfigDyn* OS_TriggerConfigDynGet(const OS_TriggerHd trigger_hd);
INLINE OS_TriggerConfigDyn* OS_TriggerConfigDynGet(const OS_TriggerHd trigger_hd)
{
const OS_ListItem* item_l_p = (OS_ListItem*)trigger_hd;
OS_TriggerConfigDyn* cfg_dyn_p = (OS_TriggerConfigDyn*)OS_ListItemValueGet(item_l_p);
    return cfg_dyn_p;
}

/******************************************************************************/
Status OS_TriggerInit(void);
Status OS_TriggerInit(void)
{
    os_trigger_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_trigger_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_triggers_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_triggers_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_TriggerCreate(const OS_TriggerConfig* cfg_p, OS_TriggerHd* trigger_hd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
const OS_TriggerHd trigger_hd = (OS_TriggerHd)item_l_p;
OS_TriggerConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_TriggerConfigDyn));
Status s = S_OK;

    if ((OS_NULL == item_l_p) || (OS_NULL == cfg_dyn_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL == trigger_hd_p) { s = S_INVALID_REF;  goto error; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_TimerHd timer_hd;
        IF_STATUS_OK(s = OS_TimerCreate(cfg_p->timer_cfg_p, &timer_hd)) {
            cfg_dyn_p->item_p   = cfg_p->item_p;
            cfg_dyn_p->state    = cfg_p->state;
            cfg_dyn_p->timer_hd = timer_hd;
            OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
            OS_ListItemOwnerSet(item_l_p, (OS_Owner)cfg_p->timer_cfg_p->id);
            OS_ListAppend(&os_triggers_list, item_l_p);
            ++triggers_count;
            if (OS_NULL != trigger_hd_p) {
                *trigger_hd_p = trigger_hd;
            }
            if (0 < cfg_p->timer_cfg_p->period) {
                IF_STATUS(s = OS_TimerStart(timer_hd, OS_TIMEOUT_DEFAULT)) {
                }
            }
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerDelete(const OS_TriggerHd trigger_hd, const TimeMs timeout)
{
OS_ListItem* item_l_p = (OS_ListItem*)trigger_hd;
Status s = S_OK;

    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, timeout)) {  // os_list protection;
        OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerDelete(timer_hd, timeout)) {
            goto error;
        }
        s = OS_TriggerItemDelete(cfg_dyn_p->item_p);
        if (S_INVALID_REF == s) { //ignore NULL storage
            s = S_OK;
        }
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        --triggers_count;
error:
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerTimerGet(const OS_TriggerHd trigger_hd, OS_TimerHd* timer_hd_p)
{
Status s = S_OK;
    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        if (OS_NULL != timer_hd_p) {
            OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
            *timer_hd_p = cfg_dyn_p->timer_hd;
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerStateGet(const OS_TriggerHd trigger_hd, OS_TriggerState* state_p)
{
Status s = S_OK;
    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        *state_p = cfg_dyn_p->state;
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerPeriodGet(const OS_TriggerHd trigger_hd, TimeMs* period_p)
{
Status s = S_OK;
    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerPeriodGet(timer_hd, period_p)) {
            OS_LOG_S(D_WARNING, s);
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerStatePeriodSet(const OS_TriggerHd trigger_hd, const TimeMs new_period, const OS_TriggerState new_state,
                                const TimeMs timeout)
{
Status s = S_OK;
    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, timeout)) {  // os_list protection;
        OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        cfg_dyn_p->state = new_state;
        IF_STATUS(s = OS_TimerPeriodSet(timer_hd, new_period, timeout)) {
            OS_LOG_S(D_WARNING, s);
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_TriggerItemCreate(const void* data_p, const U16 size, OS_TriggerItem** item_pp)
{
    return OS_StorageItemCreate(data_p, size, (OS_StorageItem**)item_pp);
}

/******************************************************************************/
Status OS_TriggerItemDelete(OS_TriggerItem* item_p)
{
    return OS_StorageItemDelete((OS_StorageItem*)item_p);
}

/******************************************************************************/
Status OS_TriggerItemOwnerAdd(OS_TriggerItem* item_p)
{
    return OS_StorageItemOwnerAdd((OS_StorageItem*)item_p);
}

/******************************************************************************/
Status OS_TriggerItemLock(OS_StorageItem* item_p, const TimeMs timeout)
{
    return OS_StorageItemLock((OS_StorageItem*)item_p, timeout);
}

/******************************************************************************/
Status OS_TriggerItemUnlock(OS_StorageItem* item_p)
{
    return OS_StorageItemUnlock((OS_StorageItem*)item_p);
}

/******************************************************************************/
OS_TriggerItem* OS_TriggerItemGet(const OS_TriggerHd trigger_hd)
{
OS_TriggerItem* item_p = OS_NULL;
    if (OS_NULL == trigger_hd) { return OS_NULL; }
    IF_STATUS_OK(OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        item_p = cfg_dyn_p->item_p;
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return item_p;
}

/******************************************************************************/
OS_TriggerItem* OS_TriggerItemByTimerIdGet(const OS_TimerId timer_id)
{
OS_TriggerItem* item_p = OS_NULL;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_ListItem* item_l_p = OS_ListItemByOwnerGet(&os_triggers_list, (OS_Owner)timer_id);
        if (OS_NULL != item_l_p) {
            const OS_TriggerConfigDyn* cfg_dyn_p = (OS_TriggerConfigDyn*)OS_ListItemValueGet(item_l_p);
            item_p = cfg_dyn_p->item_p;
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return item_p;
}

/******************************************************************************/
OS_TriggerItem* OS_TriggerItemByStateGet(const OS_TriggerState state)
{
OS_TriggerHd trigger_hd = OS_NULL;
    while (OS_NULL != (trigger_hd = OS_TriggerNextGet(trigger_hd))) {
        const OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        if (state == cfg_dyn_p->state) {
            return cfg_dyn_p->item_p;
        }
    }
    return OS_NULL;
}

/******************************************************************************/
Status OS_TriggerStatsGet(const OS_TriggerHd trigger_hd, OS_TriggerStats* stats_p)
{
Status s = S_OK;
    if (OS_NULL == trigger_hd) { return S_UNDEF_EVENT; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        const OS_TriggerConfigDyn* cfg_dyn_p = OS_TriggerConfigDynGet(trigger_hd);
        const OS_TimerHd timer_hd = cfg_dyn_p->timer_hd;
        IF_STATUS(s = OS_TimerStatsGet(timer_hd, &stats_p->timer_stats)) { goto error; }
        stats_p->item_p     = cfg_dyn_p->item_p;
        stats_p->state      = cfg_dyn_p->state;
error:
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    }
    return s;
}

/******************************************************************************/
OS_TriggerHd OS_TriggerNextGet(const OS_TriggerHd trigger_hd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)trigger_hd;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_trigger_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_triggers_list));
            if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_NULL;
            }
        } else {
            if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_ListItemNextGet(iter_li_p);
                if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                    iter_li_p = OS_NULL;
                }
            } else {
                iter_li_p = OS_NULL;
            }
        }
        OS_MutexRecursiveUnlock(os_trigger_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_TriggerHd)iter_li_p;
}

#endif // (OS_TRIGGERS_ENABLED)