/***************************************************************************//**
* @file    os_mailbox.h
* @brief   OS Mailbox.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_MAILBOX_H_
#define _OS_MAILBOX_H_

#include "os_list.h"
#include "os_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* \defgroup OS_Mailbox OS_Mailbox
* @{
*/
//------------------------------------------------------------------------------
typedef OS_TaskHd   OS_MessageSrc;
typedef void*       OS_MessageData;
typedef U16         OS_MessageSize;

enum {
    OS_MSG_UNDEF,
    OS_MSG_BROADCAST,
    OS_MSG_CMD,
    OS_MSG_APP = 32
};
typedef U16 OS_MessageId;

typedef struct {
    OS_MessageSrc   src;
    OS_MessageId    id;
    U16             size;
    U8              data[0];
} OS_Message;

//------------------------------------------------------------------------------
/// @brief      Create a message.
/// @param[in]  id              Message id.
/// @param[in]  data_p          Message data.
/// @param[in]  size            Message data size.
/// @param[in]  timeout         Message creation timeout.
/// @return     Message.
OS_Message*     OS_MessageCreate(const OS_MessageId id, const OS_MessageData data_p, const OS_MessageSize size, const OS_TimeMs timeout);

/// @brief      Delete the message.
/// @param[in]  msg_p           Message.
/// @return     None.
void            OS_MessageDelete(OS_Message* msg_p);

/// @brief      Emit the message.
/// @param[in]  msg_p           Message.
/// @param[in]  timeout         Message sending timeout.
/// @param[in]  priority        Message sending priority.
/// @return     #Status.
Status          OS_MessageEmit(OS_Message* msg_p, const OS_TimeMs timeout, const OS_MessagePrio priority);

/// @brief      Send the message.
/// @param[in]  qhd             Receiver (task) queue handle.
/// @param[in]  msg_p           Message.
/// @param[in]  timeout         Message sending timeout.
/// @param[in]  priority        Message sending priority.
/// @return     #Status.
Status          OS_MessageSend(const OS_QueueHd qhd, const OS_Message* msg_p, const OS_TimeMs timeout, const OS_MessagePrio priority);

/// @brief      Send the multicast message.
/// @param[in]  qhd_v           Vector of receiver (tasks) queue handles with trailling OS_NULL;
/// @param[in]  msg_p           Message.
/// @param[in]  timeout         Message sending timeout.
/// @param[in]  priority        Message sending priority.
/// @return     #Status.
Status          OS_MessageMulticastSend(const OS_List* slots_qhd_l_p, OS_Message* msg_p, const OS_TimeMs timeout, const OS_MessagePrio priority);

//Status          OS_MessageBroadcastSend(const OS_Message* msg_p, const OS_TimeMs timeout, const OS_MessagePrio priority);

/// @brief      Receive the message.
/// @param[in]  qhd             Receiver (task) queue handle.
/// @param[out] msg_pp          Message.
/// @param[in]  timeout         Message receiving timeout.
/// @return     #Status.
Status          OS_MessageReceive(const OS_QueueHd qhd, OS_Message** msg_pp, const OS_TimeMs timeout);

/**
* \addtogroup OS_ISR_Mailbox ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Create a message.
/// @param[in]  id              Message id.
/// @param[in]  data_p          Message data.
/// @param[in]  size            Message data size.
/// @return     Message.
OS_Message*     OS_ISR_MessageCreate(const OS_MessageSrc src, const OS_MessageId id, const OS_MessageData data_p, const OS_MessageSize size);

/// @brief      Send the message.
/// @param[in]  qhd             Receiver (task) queue handle.
/// @param[in]  msg_p           Message.
/// @param[in]  timeout         Message sending timeout.
/// @param[in]  priority        Message sending priority.
/// @return     #Status.
Status          OS_ISR_MessageSend(const OS_QueueHd qhd, const OS_Message* msg_p, const OS_MessagePrio priority);

/// @brief      Receive the message.
/// @param[in]  qhd             Receiver (task) queue handle.
/// @param[out] msg_pp          Message.
/// @param[in]  timeout         Message receiving timeout.
/// @return     #Status.
Status          OS_ISR_MessageReceive(const OS_QueueHd qhd, OS_Message** msg_pp);

/**@}*/ //OS_ISR_Mailbox

/**@}*/ //OS_Mailbox

#ifdef __cplusplus
}
#endif

#endif // _OS_MAILBOX_H_
