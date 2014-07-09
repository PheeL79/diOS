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
#include "os_message.h"
#include "os_driver.h"

//------------------------------------------------------------------------------
typedef struct {
    OS_DriverConfig         cfg;
    OS_MutexHd              mutex;
    OS_DriverStats          stats;
} OS_DriverConfigDyn;
//OS_DriverConfigDyn == OS_DriverHd(user can't access hidden fields of structure
//since it is defined as 'void*' in interface *.h file)

//------------------------------------------------------------------------------
static OS_List os_drivers_list;
static OS_MutexHd os_driver_mutex;

//------------------------------------------------------------------------------
const HAL_DriverItf* OS_DriverItfGet(const OS_DriverHd dhd);
//static OS_DriverConfigDyn* OS_DriverConfigDynGet(const OS_DriverHd dhd);

/******************************************************************************/
Status OS_DriverInit_(void);
Status OS_DriverInit_(void)
{
    os_driver_mutex = OS_MutexCreate();
    if (OS_NULL == os_driver_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_drivers_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_drivers_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
const HAL_DriverItf* OS_DriverItfGet(const OS_DriverHd dhd)
{
    return ((OS_DriverConfigDyn*)dhd)->cfg.itf_p;
}

/******************************************************************************/
//OS_DriverConfigDyn* OS_DriverConfigDynGet(const OS_DriverHd dhd)
//{
//    return (OS_DriverConfigDyn*)dhd;
//}

/******************************************************************************/
OS_TaskHd OS_DriverParentGet(const OS_DriverHd dhd)
{
const OS_ListItem* item_l_p = OS_ListItemByValueFind(&os_drivers_list, (OS_Value)dhd);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    return (OS_TaskHd)OS_LIST_ITEM_OWNER_GET(item_l_p);
}

/******************************************************************************/
OS_DriverHd OS_DriverByNameGet(ConstStrPtr name_p)
{
OS_DriverHd dhd = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_drivers_list protection;
        OS_ListItem* iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_drivers_list);
        OS_DriverConfigDyn* drv_p;

        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) {
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            drv_p = (OS_DriverConfigDyn*)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            if (!strcmp((const char*)name_p, (char*)drv_p->cfg.name)) {
                dhd = (OS_DriverHd)drv_p;
                break;
            }
        }
    } OS_MutexUnlock(os_driver_mutex);
    return dhd;
}

/******************************************************************************/
Status OS_DriverCreate(const OS_DriverConfig* cfg_p, OS_DriverHd* dhd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
OS_DriverConfigDyn* drv_p = OS_Malloc(sizeof(OS_DriverConfigDyn));
Status s = S_OK;

    if ((OS_NULL == cfg_p) || (OS_NULL == dhd_p)) { s = S_INVALID_REF;  goto error; }
    if ((OS_NULL == item_l_p) || (OS_NULL == drv_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL == cfg_p->itf_p) { s = S_INVALID_REF;  goto error; }
    IF_STATUS_OK(s = OS_MutexLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_drivers_list protection;
        *dhd_p = (OS_DriverHd)drv_p;
        memmove(&drv_p->cfg, cfg_p, sizeof(drv_p->cfg));
        memset(&drv_p->stats, 0, sizeof(OS_DriverStats));
        drv_p->stats.state      = OS_DRV_STATE_UNDEF;
        drv_p->stats.power      = PWR_UNDEF;
        drv_p->stats.status_last= s;
        drv_p->mutex = OS_MutexCreate();
        if (OS_NULL == drv_p->mutex) { s = S_INVALID_REF; goto error; }
        IF_STATUS(s) {
            drv_p->stats.errors_cnt++;
        }
        OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)drv_p);
        OS_LIST_ITEM_OWNER_SET(item_l_p, OS_TaskHdGet());
        OS_ListAppend(&os_drivers_list, item_l_p);
    }
error:
    IF_STATUS(s) {
        OS_Free(drv_p);
        OS_ListItemDelete(item_l_p);
    } OS_MutexUnlock(os_driver_mutex);
    return s;
}

/******************************************************************************/
Status OS_DriverDelete(const OS_DriverHd dhd)
{
OS_ListItem* item_l_p;
OS_DriverConfigDyn* drv_p;
Status s = S_OK;

    IF_STATUS_OK(s = OS_MutexLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_drivers_list protection;
        item_l_p = OS_ListItemByValueFind(&os_drivers_list, (OS_Value)dhd);
        if (OS_NULL == item_l_p) { s = S_INVALID_REF; goto error; }
        drv_p = (OS_DriverConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
        if ((OS_NULL == drv_p) || (OS_DELAY_MAX == (OS_Value)drv_p)) {
            s = S_INVALID_REF;
            goto error;
        }
        OS_ListItemDelete(item_l_p);
        OS_MutexDelete(drv_p->mutex);
        OS_Free(drv_p);
    }
error:
    OS_MutexUnlock(os_driver_mutex);
    return s;
}

/******************************************************************************/
Status OS_DriverInit(const OS_DriverHd dhd)
{
OS_DriverConfigDyn*  drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;

    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE == BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_INIT; }
    OS_ASSERT(OS_NULL != itf_p->Init);
    IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS_OK(s = itf_p->Init()) {
            BIT_SET(drv_p->stats.state, BIT(OS_DRV_STATE_IS_INIT));
        }
    } OS_MutexUnlock(drv_p->mutex);
    return s;
}

/******************************************************************************/
Status OS_DriverDeInit(const OS_DriverHd dhd)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_ISNT_INITED; }
    if (OS_TRUE == BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverClose(dhd)) { return s; }
    }
    OS_ASSERT(OS_NULL != itf_p->DeInit);
    IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS_OK(s = itf_p->DeInit()) {
            BIT_CLEAR(drv_p->stats.state, BIT(OS_DRV_STATE_IS_INIT));
        }
    } OS_MutexUnlock(drv_p->mutex);
    return s;
}

/******************************************************************************/
Status OS_DriverOpen(const OS_DriverHd dhd, void* args_p)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_INIT))) { return S_ISNT_INITED; }
    OS_ASSERT(OS_NULL != itf_p->Open);
    IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS_OK(s = itf_p->Open(args_p)) {
            BIT_SET(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN));
            drv_p->stats.owners++;
        }
    } OS_MutexUnlock(drv_p->mutex);
    IF_STATUS_OK(s) {
        if (1 == drv_p->stats.owners) {
            if (OS_NULL != itf_p->IoCtl) {
                const OS_PowerState state = PWR_ON;
                if (state != drv_p->stats.power) {
                    IF_STATUS_OK(s = itf_p->IoCtl(DRV_REQ_STD_POWER, (void*)&state)) {
                        drv_p->stats.power = state;
                    } else {
                        drv_p->stats.power = PWR_UNDEF;
                    }
                }
            }
        }
    }
    return s;
}

/******************************************************************************/
Status OS_DriverClose(const OS_DriverHd dhd)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    OS_ASSERT(OS_NULL != itf_p->Close);
    IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        IF_STATUS_OK(s = itf_p->Close()) {
            BIT_CLEAR(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN));
            drv_p->stats.owners--;
        }
    } OS_MutexUnlock(drv_p->mutex);
    IF_STATUS_OK(s) {
        if (0 == drv_p->stats.owners) {
            if (OS_NULL != itf_p->IoCtl) {
                const OS_PowerState state = PWR_OFF;
                if (state != drv_p->stats.power) {
                    IF_STATUS_OK(s = itf_p->IoCtl(DRV_REQ_STD_POWER, (void*)&state)) {
                        drv_p->stats.power = state;
                    } else {
                        drv_p->stats.power = PWR_UNDEF;
                    }
                }
            }
        }
    }
    return s;
}

/******************************************************************************/
Status OS_DriverRead(const OS_DriverHd dhd, void* data_in_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    if (OS_NULL != itf_p->Read) {
        IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            IF_STATUS(s = itf_p->Read(data_in_p, size, args_p)) {
                drv_p->stats.status_last = s;
                drv_p->stats.errors_cnt++;
            } else {
                drv_p->stats.received += size;
            }
        } OS_MutexUnlock(drv_p->mutex);
    } else {
        s = S_UNDEF_FUNCTION;
    }
    return s;
}

/******************************************************************************/
Status OS_DriverWrite(const OS_DriverHd dhd, void* data_out_p, U32 size, void* args_p)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    if (OS_NULL != itf_p->Write) {
        IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            IF_STATUS(s = itf_p->Write(data_out_p, size, args_p)) {
                drv_p->stats.status_last = s;
                drv_p->stats.errors_cnt++;
            } else {
                drv_p->stats.sended += size;
            }
        } OS_MutexUnlock(drv_p->mutex);
    } else {
        s = S_UNDEF_FUNCTION;
    }
    return s;
}

/******************************************************************************/
Status OS_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s = S_OK;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    if (OS_NULL != itf_p->IoCtl) {
        IF_STATUS_OK(s = OS_MutexLock(drv_p->mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            if (DRV_REQ_STD_POWER == request_id) {
                const OS_PowerState state = *(OS_PowerState*)args_p;
                if (state != drv_p->stats.power) {
                    IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                        drv_p->stats.power = PWR_UNDEF;
                        drv_p->stats.status_last = s;
                        drv_p->stats.errors_cnt++;
                    } else {
                        drv_p->stats.power = state;
                    }
                }
            } else {
                IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                    drv_p->stats.status_last = s;
                    drv_p->stats.errors_cnt++;
                }
            }
        } OS_MutexUnlock(drv_p->mutex);
    } else {
        s = S_UNDEF_FUNCTION;
    }
    return s;
}

/******************************************************************************/
Status OS_ISR_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p)
{
OS_DriverConfigDyn* drv_p = (OS_DriverConfigDyn*)dhd;
const HAL_DriverItf* itf_p = drv_p->cfg.itf_p;
Status s;
    if (OS_NULL == drv_p) { return S_INVALID_REF; }
    if (OS_TRUE != BIT_TEST(drv_p->stats.state, BIT(OS_DRV_STATE_IS_OPEN))) { return S_ISNT_OPENED; }
    if (OS_NULL != itf_p->IoCtl) {
        IF_STATUS_OK(s = OS_ISR_MutexLock(drv_p->mutex)) {
            if (DRV_REQ_STD_POWER == request_id) {
                const OS_PowerState state = *(OS_PowerState*)args_p;
                if (state != drv_p->stats.power) {
                    IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                        drv_p->stats.power = PWR_UNDEF;
                        drv_p->stats.status_last = s;
                        drv_p->stats.errors_cnt++;
                    } else {
                        drv_p->stats.power = state;
                    }
                }
            } else {
                IF_STATUS(s = itf_p->IoCtl(request_id, args_p)) {
                    drv_p->stats.status_last = s;
                    drv_p->stats.errors_cnt++;
                }
            }
        } OS_ISR_MutexUnlock(drv_p->mutex);
    } else {
        s = S_UNDEF_FUNCTION;
        //OS_ISR_LOG_S(D_WARNING, s);
    }
    return s;
}

/******************************************************************************/
ConstStrPtr OS_DriverNameGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_NULL; }
    return (ConstStrPtr)((OS_DriverConfigDyn*)dhd)->cfg.name;
}

/******************************************************************************/
ConstStrPtr OS_DriverStateNameGet(const OS_DriverState state)
{
static ConstStr undef_str[]= "undef";
static ConstStr init_str[] = "init";
static ConstStr open_str[] = "open";
ConstStrPtr state_str = undef_str;

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
    const OS_DriverConfigDyn* drv_p = (const OS_DriverConfigDyn*)dhd;
    return drv_p->stats.state;
}

/******************************************************************************/
Status OS_DriverStatsGet(const OS_DriverHd dhd, OS_DriverStats* stats_p)
{
    if ((OS_NULL == dhd) || (OS_NULL == stats_p)) { return S_INVALID_REF; }
    const OS_DriverConfigDyn* drv_p = (const OS_DriverConfigDyn*)dhd;
    memmove(stats_p, &drv_p->stats, sizeof(drv_p->stats));
    return S_OK;
}

/******************************************************************************/
const OS_DriverConfig* OS_DriverConfigGet(const OS_DriverHd dhd)
{
    if (OS_NULL == dhd) { return OS_NULL; }
    return (&((const OS_DriverConfigDyn*)dhd)->cfg);
}

/******************************************************************************/
void OS_DriverPowerPrioritySort(const SortDirection sort_dir);
void OS_DriverPowerPrioritySort(const SortDirection sort_dir)
{
    IF_STATUS_OK(OS_MutexLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_tasks_list protection;
        //TODO(A.Filyanov) Optimize selective sort function.
        OS_ListItem* item_curr_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_drivers_list));
        OS_ListItem* item_next_p;
        OS_ListItem* item_min_p;
        OS_DriverConfigDyn* cfg_dyn_min_p;
        OS_DriverConfigDyn* cfg_dyn_next_p;

        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(item_curr_p)) {
            item_min_p = item_curr_p;
            item_next_p = OS_LIST_ITEM_NEXT_GET(item_curr_p);
            while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(item_next_p)) {
                cfg_dyn_min_p  = (OS_DriverConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_min_p);
                cfg_dyn_next_p = (OS_DriverConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_next_p);

                if (SORT_ASCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg.prio_power < cfg_dyn_min_p->cfg.prio_power) {
                        item_min_p = item_next_p;
                    }
                } else if (SORT_DESCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg.prio_power > cfg_dyn_min_p->cfg.prio_power) {
                        item_min_p = item_next_p;
                    }
                } else { OS_ASSERT(OS_FALSE); }

                item_next_p = OS_LIST_ITEM_NEXT_GET(item_next_p);
            }
            if (item_curr_p != item_min_p) {
                OS_ListItemsSwap(item_curr_p, item_min_p);
            }
            item_curr_p = OS_LIST_ITEM_NEXT_GET(item_min_p);
        }
    } OS_MutexUnlock(os_driver_mutex);
}

/******************************************************************************/
OS_DriverHd OS_DriverNextGet(const OS_DriverHd dhd)
{
OS_ListItem* iter_li_p;
OS_DriverHd  drv_hd = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_driver_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_drivers_list protection;
        if (OS_NULL == dhd) {
            iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_drivers_list);
            if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) { goto error; }
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            drv_hd = (OS_DriverHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
        } else {
            iter_li_p = OS_ListItemByValueFind(&os_drivers_list, (OS_Value)dhd);
            if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) { goto error; }
                drv_hd = (OS_DriverHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            }
        }
    }
error:
    OS_MutexUnlock(os_driver_mutex);
    return drv_hd;
}