/***************************************************************************//**
* @file    os_queue.c
* @brief   OS Queue.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "common.h"
#include "os_common.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_time.h"
#include "os_list.h"
#include "os_memory.h"
#include "os_task.h"

//------------------------------------------------------------------------------
typedef struct {
    QueueHandle_t   handle;
    OS_QueueConfig  cfg;
#if (1 == OS_STATS_ENABLED)
    OS_QueueStats   stats;
#endif // OS_STATS_ENABLED
} OS_QueueConfigDyn;

//------------------------------------------------------------------------------
static OS_List os_queues_list;
static OS_MutexHd os_queue_mutex;
static volatile U32 queues_count = 0;

/******************************************************************************/
Status OS_QueueInit(void);
Status OS_QueueInit(void)
{
    os_queue_mutex = OS_MutexCreate();
    if (OS_NULL == os_queue_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_queues_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_queues_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_QueueCreate(const OS_QueueConfig* cfg_p, OS_TaskHd parent_thd, OS_QueueHd* qhd_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
OS_QueueConfigDyn* que_p = OS_Malloc(sizeof(OS_QueueConfigDyn));
Status s = S_OK;

    if ((OS_NULL == item_l_p) || (OS_NULL == que_p)) { s = S_NO_MEMORY; goto error; }
    if (OS_NULL == qhd_p) { s = S_INVALID_REF;  goto error; }
    if ((DIR_IN > cfg_p->dir) || (DIR_OUT < cfg_p->dir)) { s = S_INVALID_VALUE;  goto error; }
    IF_STATUS_OK(s = OS_MutexLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_queues_list protection;
        if (OS_NULL == (que_p->handle = xQueueCreate(cfg_p->len, cfg_p->item_size))) {
            s = S_UNDEF_QUEUE;
            goto error;
        }
        *qhd_p                      = (OS_QueueHd)que_p;
        que_p->cfg.dir              = cfg_p->dir;
        que_p->cfg.len              = cfg_p->len;
        que_p->cfg.item_size        = cfg_p->item_size;
        que_p->stats.received       = 0;
        que_p->stats.sended         = 0;
        OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)que_p);
        OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)parent_thd);
        OS_ListAppend(&os_queues_list, item_l_p);
        ++queues_count;
    }
error:
    IF_STATUS(s) {
        OS_Free(que_p);
        OS_ListItemDelete(item_l_p);
    } OS_MutexUnlock(os_queue_mutex);
    return s;
}

/******************************************************************************/
Status OS_QueueDelete(const OS_QueueHd qhd)
{
OS_ListItem* item_l_p;
OS_QueueConfigDyn* que_p;
Status s = S_OK;

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    IF_STATUS_OK(s = OS_MutexLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_queues_list protection;
        item_l_p = OS_ListItemByValueFind(&os_queues_list, (OS_Value)qhd);
        if (OS_NULL == item_l_p) { s = S_INVALID_REF; goto error; }
        que_p = (OS_QueueConfigDyn*)OS_LIST_ITEM_VALUE_GET(item_l_p);
        if ((OS_NULL == que_p) || (OS_DELAY_MAX == (OS_Value)que_p)) {
            s = S_INVALID_REF;
            goto error;
        }
        vQueueDelete(que_p->handle);
        OS_ListItemDelete(item_l_p);
        OS_Free(que_p);
        --queues_count;
    }
error:
    OS_MutexUnlock(os_queue_mutex);
    return s;
}

/******************************************************************************/
Status OS_QueueReceive(const OS_QueueHd qhd, void* item_p, const TimeMs timeout)
{
const OS_Tick ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    OS_QueueConfigDyn* que_p = (OS_QueueConfigDyn*)qhd;
    if (pdTRUE != xQueueReceive(que_p->handle, item_p, ticks)) {
        return S_MODULE;
    }
    que_p->stats.received++;
    return S_OK;
}

/******************************************************************************/
Status OS_QueueSend(const OS_QueueHd qhd, const void* item_p, const TimeMs timeout, const OS_MessagePrio priority)
{
const OS_Tick ticks = ((OS_BLOCK == timeout) || (OS_NO_BLOCK == timeout)) ? timeout : OS_MS_TO_TICKS(timeout);
OS_Status os_s;
Status s = S_OK;

    if (OS_NULL != qhd) {
        OS_QueueConfigDyn* que_p = (OS_QueueConfigDyn*)qhd;
        if (OS_NULL != que_p->handle) {
            if (OS_MSG_PRIO_HIGH == priority) {
                os_s = xQueueSendToFront(que_p->handle, item_p, ticks);
            } else if (OS_MSG_PRIO_NORMAL == priority) {
                os_s = xQueueSendToBack(que_p->handle, item_p, ticks);
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
            }
            que_p->stats.sended++;
        } else {
            s = S_UNDEF_QUEUE;
        }
    } else {
        s = S_UNDEF_QUEUE;
    }
    return s;
}

/******************************************************************************/
Status OS_QueueFlush(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    xQueueReset(((OS_QueueConfigDyn*)qhd)->handle);
    return S_OK;
}

/******************************************************************************/
U32 OS_QueueItemsCountGet(const OS_QueueHd qhd)
{
    if (OS_NULL == qhd) { return OS_DELAY_MAX; }
    return (U32)uxQueueMessagesWaiting(((OS_QueueConfigDyn*)qhd)->handle);
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
    memcpy(config_p, &((OS_QueueConfigDyn*)qhd)->cfg, sizeof(((OS_QueueConfigDyn*)qhd)->cfg));
    return s;
}

/******************************************************************************/
Status OS_QueueStatsGet(const OS_QueueHd qhd, OS_QueueStats* stats_p)
{
Status s = S_OK;
    if ((OS_NULL == qhd) || (OS_NULL == stats_p)) { return S_INVALID_REF; }
    memcpy(stats_p, &((OS_QueueConfigDyn*)qhd)->stats, sizeof(((OS_QueueConfigDyn*)qhd)->stats));
    return s;
}

/******************************************************************************/
OS_TaskHd OS_QueueParentGet(const OS_QueueHd qhd)
{
const OS_ListItem* item_l_p = OS_ListItemByValueFind(&os_queues_list, (OS_Value)qhd);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    return (OS_TaskHd)OS_LIST_ITEM_OWNER_GET(item_l_p);
}

/******************************************************************************/
OS_QueueHd OS_QueueNextGet(const OS_QueueHd qhd)
{
OS_ListItem* iter_li_p;
OS_QueueHd queue_hd = OS_NULL;
    IF_STATUS_OK(OS_MutexLock(os_queue_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_queues_list protection;
        if (OS_NULL == qhd) {
            iter_li_p = (OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_queues_list);
            if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(iter_li_p))) { goto error; }
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            queue_hd = (OS_QueueHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
        } else {
            iter_li_p = OS_ListItemByValueFind(&os_queues_list, (OS_Value)qhd);
            if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
                if (OS_DELAY_MAX == OS_LIST_ITEM_VALUE_GET(iter_li_p)) { goto error; }
                queue_hd = (OS_QueueHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            }
        }
    }
error:
    OS_MutexUnlock(os_queue_mutex);
    return queue_hd;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_QueueReceive(const OS_QueueHd qhd, void* item_p)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (OS_NULL == qhd) { return S_UNDEF_QUEUE; }
    OS_QueueConfigDyn* que_p = (OS_QueueConfigDyn*)qhd;
    if (pdTRUE != xQueueReceiveFromISR(que_p->handle, item_p, &xHigherPriorityTaskWoken)) {
        return S_MODULE;
    }
    que_p->stats.received++;
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
        OS_QueueConfigDyn* que_p = (OS_QueueConfigDyn*)qhd;
        if (OS_MSG_PRIO_HIGH == priority) {
            os_s = xQueueSendToFrontFromISR(que_p->handle, item_p, &xHigherPriorityTaskWoken);
        } else if (OS_MSG_PRIO_NORMAL == priority) {
            os_s = xQueueSendToBackFromISR(que_p->handle, item_p, &xHigherPriorityTaskWoken);
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
            que_p->stats.sended++;
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
    return (U32)uxQueueMessagesWaitingFromISR(((OS_QueueConfigDyn*)qhd)->handle);
}
