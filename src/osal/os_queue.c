/***************************************************************************//**
* @file    os_queue.c
* @brief   OS Queue.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "os_common.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_time.h"
#include "os_list.h"
#include "os_memory.h"
#include "os_task.h"

//------------------------------------------------------------------------------
typedef struct {
    OS_TaskHd       parent_thd;
    OS_QueueConfig  cfg;
#if (1 == OS_STATS_ENABLED)
    OS_QueueStats   stats;
#endif // OS_STATS_ENABLED
} OS_QueueConfigDyn;

//------------------------------------------------------------------------------
static OS_List os_queues_list;
static OS_MutexHd os_queue_mutex;
static U32 queues_count = 0;

/******************************************************************************/
Status OS_QueueInit(void);
Status OS_QueueInit(void)
{
    os_queue_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_queue_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_queues_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_queues_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_QueueCreate(const OS_QueueConfig* cfg_p, OS_TaskHd parent_thd, OS_QueueHd* qhd_p)
{
Status s = S_OK;
    if (OS_NULL == qhd_p) { return S_INVALID_REF; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_QueueConfigDyn* cfg_dyn_p= OS_Malloc(sizeof(OS_QueueConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    const QueueHandle_t queue_hd = xQueueCreate(cfg_p->len, cfg_p->item_size);
    if (OS_NULL == queue_hd) { s = S_UNDEF_QUEUE; goto error; }
    *qhd_p                          = (OS_QueueHd)item_l_p;
    cfg_dyn_p->parent_thd           = parent_thd;
    cfg_dyn_p->cfg.len              = cfg_p->len;
    cfg_dyn_p->cfg.item_size        = cfg_p->item_size;
    cfg_dyn_p->stats.received       = 0;
    cfg_dyn_p->stats.sended         = 0;
    OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
    OS_ListItemOwnerSet(item_l_p, (OS_Owner)queue_hd);
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        ++queues_count;
        OS_ListAppend(&os_queues_list, item_l_p);
        OS_MutexRecursiveUnlock(os_queue_mutex);
    }
error:
    IF_STATUS(s) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    }
    return s;
}

/******************************************************************************/
Status OS_QueueDelete(const OS_QueueHd qhd)
{
Status s = S_OK;

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* item_l_p = (OS_ListItem*)qhd;
        OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet(item_l_p);
        vQueueDelete((QueueHandle_t)OS_ListItemOwnerGet(item_l_p));
        OS_ListItemDelete(item_l_p);
        OS_Free(cfg_dyn_p);
        --queues_count;
        OS_MutexRecursiveUnlock(os_queue_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_QueueReceive(const OS_QueueHd qhd, void* item_p, const TimeMs timeout)
{
const OS_Tick ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
    OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
    if (pdTRUE != xQueueReceive(queue_hd, item_p, ticks)) {
        return S_MODULE;
    }
    cfg_dyn_p->stats.received++;
    return S_OK;
}

/******************************************************************************/
Status OS_QueueSend(const OS_QueueHd qhd, const void* item_p, const TimeMs timeout, const OS_MessagePrio priority)
{
const OS_Tick ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
OS_Status os_s;
Status s = S_OK;

    if (OS_NULL != qhd) {
        QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
        OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
        if (OS_MSG_PRIO_HIGH == priority) {
            os_s = xQueueSendToFront(queue_hd, item_p, ticks);
        } else if (OS_MSG_PRIO_NORMAL == priority) {
            os_s = xQueueSendToBack(queue_hd, item_p, ticks);
        } else {
            Status s = S_UNDEF_PARAMETER;
            OS_LOG_S(D_WARNING, s);
            return s;
        }
        if (pdTRUE != os_s) {
            if (errQUEUE_FULL == os_s) {
                s = S_OVERFLOW;
            } else {
                s = S_MODULE;
            }
//            OS_LOG_S(D_WARNING, s);
        }
        cfg_dyn_p->stats.sended++;
    } else {
        s = S_UNDEF_QUEUE;
//        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

/******************************************************************************/
Status OS_QueueFlush(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
    xQueueReset(queue_hd);
    return S_OK;
}

/******************************************************************************/
U32 OS_QueueItemsCountGet(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return OS_DELAY_MAX; }
    QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
    return (U32)uxQueueMessagesWaiting(queue_hd);
}

/******************************************************************************/
U32 OS_QueuesCountGet(void)
{
    return queues_count;
}

/******************************************************************************/
Status OS_QueueConfigGet(const OS_QueueHd qhd, OS_QueueConfig* config_p)
{
Status s = S_OK;
    if ((OS_NULL == qhd) || (OS_NULL == config_p)) { return S_INVALID_REF; }
    OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
    OS_MemCpy(config_p, &cfg_dyn_p->cfg, sizeof(cfg_dyn_p->cfg));
    return s;
}

/******************************************************************************/
Status OS_QueueStatsGet(const OS_QueueHd qhd, OS_QueueStats* stats_p)
{
Status s = S_OK;
    if ((OS_NULL == qhd) || (OS_NULL == stats_p)) { return S_INVALID_REF; }
    OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
    OS_MemCpy(stats_p, &cfg_dyn_p->stats, sizeof(cfg_dyn_p->stats));
    return s;
}

/******************************************************************************/
OS_TaskHd OS_QueueParentGet(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return OS_NULL; }
    OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
    return cfg_dyn_p->parent_thd;
}

/******************************************************************************/
OS_QueueHd OS_QueueNextGet(const OS_QueueHd qhd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)qhd;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_queues_list));
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
        OS_MutexRecursiveUnlock(os_queue_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_QueueHd)iter_li_p;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_QueueReceive(const OS_QueueHd qhd, void* item_p)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
    OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
    if (pdTRUE != xQueueReceiveFromISR(queue_hd, item_p, &xHigherPriorityTaskWoken)) {
        return S_MODULE;
    }
    cfg_dyn_p->stats.received++;
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ISR_QueueSend(const OS_QueueHd qhd, const void* item_p, const OS_MessagePrio priority)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
OS_Status os_s;
Status s = S_OK;

    if (OS_NULL != qhd) {
        QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
        OS_QueueConfigDyn* cfg_dyn_p = (OS_QueueConfigDyn*)OS_ListItemValueGet((OS_ListItem*)qhd);
        if (OS_MSG_PRIO_HIGH == priority) {
            os_s = xQueueSendToFrontFromISR(queue_hd, item_p, &xHigherPriorityTaskWoken);
        } else if (OS_MSG_PRIO_NORMAL == priority) {
            os_s = xQueueSendToBackFromISR(queue_hd, item_p, &xHigherPriorityTaskWoken);
        } else {
            Status s = S_UNDEF_PARAMETER;
            //OS_ISR_Log(D_WARNING, s);
            return s;
        }
        if (pdTRUE != os_s) {
            if (errQUEUE_FULL == os_s) {
                s = S_OVERFLOW;
            } else {
                s = S_MODULE;
            }
        } else {
            cfg_dyn_p->stats.sended++;
            if (xHigherPriorityTaskWoken) {
                s = 1;
            }
        }
    } else {
        s = S_UNDEF_QUEUE;
    }
    return s;
}

/******************************************************************************/
U32 OS_ISR_QueueItemsCountGet(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return OS_DELAY_MAX; }
    QueueHandle_t queue_hd = (QueueHandle_t)OS_ListItemOwnerGet((OS_ListItem*)qhd);
    return (U32)uxQueueMessagesWaitingFromISR(queue_hd);
}
