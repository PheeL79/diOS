/***************************************************************************//**
* @file    os_queue.h
* @brief   OS Queue.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_QUEUE_H_
#define _OS_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_common.h"
#include "queue.h"
#include "status.h"

/**
* \defgroup OS_Queue OS_Queue
* @{
*/
//------------------------------------------------------------------------------
typedef void* OS_QueueHd;
#if (OS_QUEUE_SET_ENABLED)
typedef void* OS_QueueSetHd;
typedef void* OS_QueueSetItemHd;
#endif //(OS_QUEUE_SET_ENABLED)

typedef struct {
    U16             len;
    U16             item_size;
} OS_QueueConfig;

typedef struct {
    U32             sent;
    U32             received;
} OS_QueueStats;

//------------------------------------------------------------------------------
/// @brief      Create a queue.
/// @param[in]  cfg_p           Queue config.
/// @param[in]  parent_thd      Parent task handle.
/// @param[out] qhd_p           Queue handle.
/// @return     #Status.
Status          OS_QueueCreate(const OS_QueueConfig* cfg_p, OS_TaskHd parent_thd, OS_QueueHd* qhd_p);

/// @brief      Delete the queue.
/// @param[in]  qhd             Queue handle.
/// @return     #Status.
Status          OS_QueueDelete(const OS_QueueHd qhd);

/// @brief      Peek the item.
/// @param[in]  qhd             Receiver queue handle.
/// @param[out] item_p          Item.
/// @param[in]  timeout         Item receiving timeout.
/// @return     #Status.
Status          OS_QueuePeek(const OS_QueueHd qhd, void* item_p, const OS_TimeMs timeout);

/// @brief      Receive the item.
/// @param[in]  qhd             Receiver queue handle.
/// @param[out] item_p          Item.
/// @param[in]  timeout         Item receiving timeout.
/// @return     #Status.
Status          OS_QueueReceive(const OS_QueueHd qhd, void* item_p, const OS_TimeMs timeout);

/// @brief      Send the item.
/// @param[in]  qhd             Receiver queue handle.
/// @param[in]  item_p          Item.
/// @param[in]  timeout         Item sending timeout.
/// @param[in]  priority        Item sending priority.
/// @return     #Status.
Status          OS_QueueSend(const OS_QueueHd qhd, const void* item_p, const OS_TimeMs timeout, const OS_MessagePrio priority);

/// @brief      Clear the queue.
/// @param[in]  qhd             Queue handle.
/// @return     #Status.
Status          OS_QueueClear(const OS_QueueHd qhd);

/// @brief      Get queue items count.
/// @param[in]  qhd             Queue handle.
/// @return     Items count.
U32             OS_QueueItemsCountGet(const OS_QueueHd qhd);

/// @brief      Get queue config.
/// @param[in]  qhd             Queue handle.
/// @param[out] config_p        Queue config.
/// @return     #Status.
Status          OS_QueueConfigGet(const OS_QueueHd qhd, OS_QueueConfig* config_p);

/// @brief      Get queue statistics.
/// @param[in]  qhd             Queue handle.
/// @param[out] stats_p         Queue statistics.
/// @return     #Status.
Status          OS_QueueStatsGet(const OS_QueueHd qhd, OS_QueueStats* stats_p);

/// @brief      Get queue parent.
/// @param[in]  qhd             Queue handle.
/// @return     Task handle.
OS_TaskHd       OS_QueueParentGet(const OS_QueueHd qhd);

/// @brief      Get system supervise task standart input/output queue.
/// @return     Queue handle.
OS_QueueHd      OS_QueueSvStdInGet(void);

/// @brief      Get system queues count.
/// @return     Queues count.
U32             OS_QueuesCountGet(void);

/// @brief      Get the next queue.
/// @param[in]  qhd             Queue handle.
/// @return     Queue handle.
OS_QueueHd      OS_QueueNextGet(const OS_QueueHd qhd);

#if (OS_QUEUE_SET_ENABLED)
Status          OS_QueueSetCreate(const Size combined_size, OS_QueueSetHd* qshd_p);

Status          OS_QueueSetDelete(const OS_QueueSetHd qshd);

Status          OS_QueueSetItemAppend(const OS_QueueSetHd qshd, const OS_QueueSetItemHd item_qshd);

Status          OS_QueueSetItemRemove(const OS_QueueSetHd qshd, const OS_QueueSetItemHd item_qshd);

OS_QueueSetItemHd OS_QueueSetReceive(const OS_QueueSetHd qshd, const OS_TimeMs timeout);
#endif //(OS_QUEUE_SET_ENABLED)

/**
* \addtogroup OS_ISR_Queue ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Receive the item.
/// @param[in]  qhd             Receiver queue handle.
/// @param[out] item_p          Item.
/// @return     #Status.
/// @return     0 - OK
/// \n          1 - OK, needs to context switch (reading from queue unblock the task waiting for room in this one).
/// \n          < 0 - error #Status.
Status          OS_ISR_QueueReceive(const OS_QueueHd qhd, void* item_p);

/// @brief      Send the item.
/// @param[in]  qhd             Receiver queue handle.
/// @param[in]  item_p          Item.
/// @param[in]  priority        Item sending priority.
/// @return     #Status.
/// @return     0 - OK
/// \n          1 - OK, needs to context switch (reading from queue unblock the task waiting for room in this one).
/// \n          < 0 - error #Status.
Status          OS_ISR_QueueSend(const OS_QueueHd qhd, const void* item_p, const OS_MessagePrio priority);

/// @brief      Get queue items count.
/// @param[in]  qhd             Queue handle.
/// @return     Items count.
U32             OS_ISR_QueueItemsCountGet(const OS_QueueHd qhd);

#if (OS_QUEUE_SET_ENABLED)
OS_QueueSetItemHd OS_ISR_QueueSetReceive(const OS_QueueSetHd qshd);
#endif //(OS_QUEUE_SET_ENABLED)

/**@}*/ //OS_ISR_Queue

/**@}*/ //OS_Queue

#ifdef __cplusplus
}
#endif

#endif // _OS_QUEUE_H_
