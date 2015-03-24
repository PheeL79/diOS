/***************************************************************************//**
* @file    os_driver.c
* @brief   OS Driver.
* @author  A. Filyanov
*******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include "osal.h"
#include "os_debug.h"
#include "os_mutex.h"
#include "os_list.h"
#include "os_memory.h"
#include "os_mailbox.h"
#include "os_driver.h"

//------------------------------------------------------------------------------
typedef struct {
    OS_DriverConfig         cfg;
    OS_MutexHd              mutex;
    OS_DriverStats          stats;
} OS_DriverConfigDyn;

//------------------------------------------------------------------------------
static OS_List os_drivers_list;
static OS_MutexHd os_driver_mutex;

/******************************************************************************/
static OS_DriverConfigDyn* OS_DriverConfigDynGet(const OS_DriverHd dhd);
INLINE OS_DriverConfigDyn* OS_DriverConfigDynGet(const OS_DriverHd dhd)
{
    OS_ASSERT_VALUE(OS_NULL != dhd);
    const OS_ListItem* item_l_p = (OS_ListItem*)dhd;
    OS_DriverConfigDyn* cfg_dyn_p = (OS_DriverConfigDyn*)OS_ListItemValueGet(item_l_p);
    OS_ASSERT_VALUE(OS_DELAY_MAX != (OS_Value)cfg_dyn_p);
    return cfg_dyn_p;
}

/******************************************************************************/
Status OS_DriverInit_(void);
Status OS_DriverInit_(void)
{
    os_driver_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_driver_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_drivers_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_drivers_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
OS_TaskHd OS_DriverParentGet(const OS_DriverHd dhd)
{
const OS_ListItem* item_l_p = OS_ListItemByValueGet(&os_drivers_list, (OS_Value)dhd);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    return (OS_TaskHd)OS_ListItemOwnerGet(item_l_p);
}

/******************************************************************************/
U16 OS_DriverOwnersCountGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_DRV_STATE_UNDEF; }
    const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    return cfg_dyn_p->stats.owners;
}

/******************************************************************************/
OS_DriverHd OS_DriverByNameGet(ConstStrP name_p)
{
OS_ListItem* iter_li_p = OS_NULL;
    IF_OK(OS_MutexRecursiveLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_drivers_list));
        if (OS_NULL == name_p) {
            goto exit;
        }
        if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
            do {
                const OS_DriverConfigDyn* cfg_dyn_p = (OS_DriverConfigDyn*)OS_ListItemValueGet(iter_li_p);
                if (!OS_StrCmp((const char*)name_p, (char*)cfg_dyn_p->cfg.name)) {
                    goto exit;
                }
                iter_li_p = OS_ListItemNextGet(iter_li_p);
            } while (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p));
        }
        iter_li_p = OS_NULL;
exit:
        OS_MutexRecursiveUnlock(os_driver_mutex);
    }
    return (OS_DriverHd)iter_li_p;
}

/******************************************************************************/
Status OS_DriverCreate(const OS_DriverConfig* cfg_p, OS_DriverHd* dhd_p)
{
Status s = S_OK;

    if (OS_NULL == cfg_p) { return S_INVALID_REF; }
    if (OS_NULL == cfg_p->itf_p) { return S_INVALID_REF; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_DriverConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_DriverConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    OS_MemMov(&cfg_dyn_p->cfg, cfg_p, sizeof(cfg_dyn_p->cfg));
    OS_MemSet(&cfg_dyn_p->stats, 0, sizeof(OS_DriverStats));
    cfg_dyn_p->stats.state      = OS_DRV_STATE_UNDEF;
    cfg_dyn_p->stats.power      = PWR_UNDEF;
    cfg_dyn_p->stats.status_last= s;
    cfg_dyn_p->mutex = OS_MutexCreate();
    if (OS_NULL == cfg_dyn_p->mutex) { s = S_INVALID_REF; goto error; }
    OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
    OS_ListItemOwnerSet(item_l_p, (OS_Owner)OS_TaskGet());
    IF_OK(s = OS_MutexRecursiveLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListAppend(&os_drivers_list, item_l_p);
        OS_MutexRecursiveUnlock(os_driver_mutex);
    }
    if (OS_NULL != dhd_p) {
        *dhd_p = (OS_DriverHd)item_l_p;
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverDelete(const OS_DriverHd dhd)
{
Status s = S_OK;

    IF_OK(s = OS_MutexRecursiveLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* item_l_p = (OS_ListItem*)dhd;
        OS_DriverConfigDyn* cfg_dyn_p = (OS_DriverConfigDyn*)OS_ListItemValueGet(item_l_p);
        OS_ListItemDelete(item_l_p);
        OS_MutexDelete(cfg_dyn_p->mutex);
        OS_Free(cfg_dyn_p);
        OS_MutexRecursiveUnlock(os_driver_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverInit(const OS_DriverHd dhd, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    if (OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_INIT; }
    OS_ASSERT(OS_NULL != itf_p->Init);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_OK(s = itf_p->Init(args_p)) {
            BIT_SET(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT));
        }
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverDeInit(const OS_DriverHd dhd, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    if (OS_TRUE != BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_ISNT_INITED; }
    if (OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverClose(dhd, args_p)) { return s; }
    }
    OS_ASSERT(OS_NULL != itf_p->DeInit);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_OK(s = itf_p->DeInit(args_p)) {
            BIT_CLEAR(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT));
        }
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverOpen(const OS_DriverHd dhd, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    if (OS_TRUE != BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_ISNT_INITED; }
    OS_ASSERT(OS_NULL != itf_p->Open);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_OK(s) {
            if (0 == cfg_dyn_p->stats.owners) {
                if (OS_NULL != itf_p->IoCtl) {
                    const OS_PowerState state = PWR_ON;
                    if (state != cfg_dyn_p->stats.power) {
                        IF_OK(s = itf_p->IoCtl(DRV_REQ_STD_POWER_SET, (void*)&state)) {
                            cfg_dyn_p->stats.power = state;
                        } else {
                            cfg_dyn_p->stats.power = PWR_UNDEF;
                        }
                    }
                }
            }
        }
        IF_OK(s = itf_p->Open(args_p)) {
            BIT_SET(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN));
            cfg_dyn_p->stats.owners++;
        } else {/*TODO(A. Filyanov) Power shutdown!*/}
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverClose(const OS_DriverHd dhd, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    if (OS_TRUE != BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    OS_ASSERT(OS_NULL != itf_p->Close);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        if (1 == cfg_dyn_p->stats.owners) {
            IF_OK(s = itf_p->Close(args_p)) {
                BIT_CLEAR(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN));
                IF_OK(s) {
                    if (OS_NULL != itf_p->IoCtl) {
                        const OS_PowerState state = PWR_OFF;
                        if (state != cfg_dyn_p->stats.power) {
                            IF_OK(s = itf_p->IoCtl(DRV_REQ_STD_POWER_SET, (void*)&state)) {
                                cfg_dyn_p->stats.power = state;
                            } else {
                                cfg_dyn_p->stats.power = PWR_UNDEF;
                            }
                        }
                    }
                }
            }
        }
        IF_OK(s) {
            cfg_dyn_p->stats.owners--;
        }
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverRead(const OS_DriverHd dhd, void* data_in_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    OS_ASSERT_VALUE(OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN)));
    OS_ASSERT_VALUE(OS_NULL != itf_p->Read);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS(s = itf_p->Read(data_in_p, size, args_p)) {
            cfg_dyn_p->stats.status_last = s;
            cfg_dyn_p->stats.errors_cnt++;
        } else {
            cfg_dyn_p->stats.received += size;
        }
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_ISR_DriverRead(const OS_DriverHd dhd, void* data_in_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    OS_ASSERT_VALUE(OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN)));
    OS_ASSERT_VALUE(OS_NULL != itf_p->Read);
    s = OS_ISR_MutexLock(cfg_dyn_p->mutex);
    if ((S_OK == s) || (1 == s)) {
        IF_STATUS(s = itf_p->Read(data_in_p, size, args_p)) {
            cfg_dyn_p->stats.status_last = s;
            cfg_dyn_p->stats.errors_cnt++;
        } else {
            cfg_dyn_p->stats.received += size;
        }
        OS_ISR_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverWrite(const OS_DriverHd dhd, void* data_out_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    OS_ASSERT_VALUE(OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN)));
    OS_ASSERT_VALUE(OS_NULL != itf_p->Write);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS(s = itf_p->Write(data_out_p, size, args_p)) {
            cfg_dyn_p->stats.status_last = s;
            cfg_dyn_p->stats.errors_cnt++;
        } else {
            cfg_dyn_p->stats.sended += size;
        }
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_ISR_DriverWrite(const OS_DriverHd dhd, void* data_out_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    OS_ASSERT_VALUE(OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN)));
    OS_ASSERT_VALUE(OS_NULL != itf_p->Write);
    s = OS_ISR_MutexLock(cfg_dyn_p->mutex);
    if ((S_OK == s) || (1 == s)) {
        IF_STATUS(s = itf_p->Write(data_out_p, size, args_p)) {
            cfg_dyn_p->stats.status_last = s;
            cfg_dyn_p->stats.errors_cnt++;
        } else {
            cfg_dyn_p->stats.sended += size;
        }
        OS_ISR_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s = S_OK;
    OS_ASSERT_VALUE(OS_NULL != itf_p->IoCtl);
    if (OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            if (DRV_REQ_STD_POWER_SET == request_id) {
                const OS_PowerState state = *(OS_PowerState*)args_p;
                if (state != cfg_dyn_p->stats.power) {
                    IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                        cfg_dyn_p->stats.power = PWR_UNDEF;
                        cfg_dyn_p->stats.status_last = s;
                        cfg_dyn_p->stats.errors_cnt++;
                    } else {
                        cfg_dyn_p->stats.power = state;
                    }
                }
            } else {
                IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                    cfg_dyn_p->stats.status_last = s;
                    cfg_dyn_p->stats.errors_cnt++;
                }
            }
            OS_MutexUnlock(cfg_dyn_p->mutex);
        }
    } else {
        s = S_ISNT_OPENED;
        OS_LOG(D_DEBUG, "%s: %s", cfg_dyn_p->cfg.name, StatusStringGet(s, MDL_STATUS_ITEMS));
    }
    return s;
}

/******************************************************************************/
Status OS_ISR_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p)
{
OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
const HAL_DriverItf* itf_p = cfg_dyn_p->cfg.itf_p;
Status s;
    OS_ASSERT_VALUE(OS_TRUE == BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN)));
    OS_ASSERT_VALUE(OS_NULL != itf_p->IoCtl);
    s = OS_ISR_MutexLock(cfg_dyn_p->mutex);
    if ((S_OK == s) || (1 == s)) {
        if (DRV_REQ_STD_POWER_SET == request_id) {
            const OS_PowerState state = *(OS_PowerState*)args_p;
            if (state != cfg_dyn_p->stats.power) {
                IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                    cfg_dyn_p->stats.power = PWR_UNDEF;
                    cfg_dyn_p->stats.status_last = s;
                    cfg_dyn_p->stats.errors_cnt++;
                } else {
                    cfg_dyn_p->stats.power = state;
                }
            }
        } else {
            IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                cfg_dyn_p->stats.status_last = s;
                cfg_dyn_p->stats.errors_cnt++;
            }
        }
        OS_ISR_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
const HAL_DriverItf* OS_DriverItfGet(const OS_DriverHd dhd)
{
const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    return cfg_dyn_p->cfg.itf_p;
}

/******************************************************************************/
Status OS_DriverItfSet(const OS_DriverHd dhd, HAL_DriverItf* itf_p)
{
Status s = S_OK;
    OS_ASSERT(OS_NULL != itf_p);
    OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    IF_OK(s = OS_MutexLock(cfg_dyn_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        OS_ASSERT_VALUE(OS_TRUE != BIT_TEST(cfg_dyn_p->stats.state, BIT(OS_DRV_STATE_IS_INIT)));
        OS_MemMov(cfg_dyn_p->cfg.itf_p, itf_p, sizeof(HAL_DriverItf));
        OS_MutexUnlock(cfg_dyn_p->mutex);
    }
    return s;
}

/******************************************************************************/
ConstStrP OS_DriverNameGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_NULL; }
    const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    return (ConstStrP)cfg_dyn_p->cfg.name;
}

/******************************************************************************/
ConstStrP OS_DriverStateNameGet(const OS_DriverState state)
{
static ConstStr undef_str[]= "undef";
static ConstStr init_str[] = "init";
static ConstStr open_str[] = "open";
ConstStrP state_str = undef_str;

    if (BIT_TEST(state, (OS_DriverState)BIT(OS_DRV_STATE_IS_INIT))) {
        state_str = init_str;
    }
    if (BIT_TEST(state, (OS_DriverState)BIT(OS_DRV_STATE_IS_OPEN))) {
        state_str = open_str;
    }
    return state_str;
}

/******************************************************************************/
OS_DriverState OS_DriverStateStateGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_DRV_STATE_UNDEF; }
    const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    return cfg_dyn_p->stats.state;
}

/******************************************************************************/
Status OS_DriverStatsGet(const OS_DriverHd dhd, OS_DriverStats* stats_p)
{
    if ((OS_NULL == dhd) || (OS_NULL == stats_p)) { return S_INVALID_REF; }
    const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    OS_MemMov(stats_p, &cfg_dyn_p->stats, sizeof(cfg_dyn_p->stats));
    return S_OK;
}

/******************************************************************************/
const OS_DriverConfig* OS_DriverConfigGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_NULL; }
    const OS_DriverConfigDyn* cfg_dyn_p = OS_DriverConfigDynGet(dhd);
    return (&(cfg_dyn_p->cfg));
}

/******************************************************************************/
void OS_DriverPowerPrioritySort(const SortDirection sort_dir);
void OS_DriverPowerPrioritySort(const SortDirection sort_dir)
{
    IF_OK(OS_MutexRecursiveLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        //TODO(A.Filyanov) Optimize selective sort function.
        OS_ListItem* item_curr_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_drivers_list));
        OS_ListItem* item_next_p;
        OS_ListItem* item_min_p;
        OS_DriverConfigDyn* cfg_dyn_min_p;
        OS_DriverConfigDyn* cfg_dyn_next_p;

        while (OS_DELAY_MAX != OS_ListItemValueGet(item_curr_p)) {
            item_min_p = item_curr_p;
            item_next_p = OS_ListItemNextGet(item_curr_p);
            while (OS_DELAY_MAX != OS_ListItemValueGet(item_next_p)) {
                cfg_dyn_min_p  = (OS_DriverConfigDyn*)OS_ListItemValueGet(item_min_p);
                cfg_dyn_next_p = (OS_DriverConfigDyn*)OS_ListItemValueGet(item_next_p);

                if (SORT_ASCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg.prio_power < cfg_dyn_min_p->cfg.prio_power) {
                        item_min_p = item_next_p;
                    }
                } else if (SORT_DESCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg.prio_power > cfg_dyn_min_p->cfg.prio_power) {
                        item_min_p = item_next_p;
                    }
                } else { OS_ASSERT(OS_FALSE); }

                item_next_p = OS_ListItemNextGet(item_next_p);
            }
            if (item_curr_p != item_min_p) {
                OS_ListItemsSwap(item_curr_p, item_min_p);
            }
            item_curr_p = OS_ListItemNextGet(item_min_p);
        }
        OS_MutexRecursiveUnlock(os_driver_mutex);
    }
}

/******************************************************************************/
OS_DriverHd OS_DriverNextGet(const OS_DriverHd dhd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)dhd;
    IF_OK(OS_MutexRecursiveLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_drivers_list));
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
        OS_MutexRecursiveUnlock(os_driver_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_DriverHd)iter_li_p;
}