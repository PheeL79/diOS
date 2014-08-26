/***************************************************************************//**
* @file    os_message.c
* @brief   OS Message.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "os_common.h"
#include "os_debug.h"
#include "os_task.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_message.h"

//------------------------------------------------------------------------------
static void SignalSend(const OS_TaskId src_tid, const Status status, const OS_SignalId signal_id);

/******************************************************************************/
OS_Message* OS_MessageCreate(const OS_MessageId id, const U16 size, const TimeMs timeout, const void* data_p)
{
OS_Message* msg_p = OS_Malloc(size + sizeof(OS_Message));

    if (msg_p) {
        OS_MEMCPY(msg_p->data, data_p, size);
        msg_p->id   = id;
        msg_p->size = size;
        msg_p->src  = OS_TaskGet();
    }
    return msg_p;
}

/******************************************************************************/
void OS_MessageDelete(OS_Message* msg_p)
{
    OS_Free(msg_p);
}

/******************************************************************************/
Status OS_MessageSend(const OS_QueueHd qhd, const OS_Message* msg_p, const TimeMs timeout, const OS_MessagePrio priority)
{
    return OS_QueueSend(qhd, &msg_p, timeout, priority);
}

/******************************************************************************/
#pragma inline
Status OS_MessageMulticastSend(const OS_List* slots_qhd_l_p, const OS_Message* msg_p, const TimeMs timeout, const OS_MessagePrio priority)
{
Status s = S_OK;
    if (OS_NULL != slots_qhd_l_p) {
        OS_ListItem* iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(slots_qhd_l_p));
        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
            const OS_QueueHd slot_qhd = (OS_QueueHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
            IF_STATUS(s = OS_QueueSend(slot_qhd, &msg_p, timeout, priority)) { return s; }
            iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
    //        if (OS_TRUE != OS_SIGNAL_IS(msg_p)) {
    //            //TODO(A. Filyanov) Copy the message on every iteration!
    //        }
        }
    } else {
        s = S_INVALID_REF;
    }
    return s;
}

/******************************************************************************/
Status OS_MessageEmit(const OS_Message* msg_p, const TimeMs timeout, const OS_MessagePrio priority)
{
extern const OS_List* OS_TaskSlotsGet(const OS_TaskHd thd);
const OS_List* slots_qhd_l_p = OS_TaskSlotsGet(OS_THIS_TASK);
    return OS_MessageMulticastSend(slots_qhd_l_p, msg_p, timeout, priority);
}

/******************************************************************************/
Status OS_MessageReceive(const OS_QueueHd qhd, OS_Message** msg_pp, const TimeMs timeout)
{
extern Status OS_TaskPowerStateSet(const OS_TaskHd thd, const OS_PowerState state);
Status s;
signal_filter: //Prevent recursion calls.
    IF_STATUS_OK(s = OS_QueueReceive(qhd, msg_pp, timeout)) {
        const OS_Message* msg_p = *msg_pp;
        if (OS_SIGNAL_IS(msg_p)) { //Filter system signals.
            const OS_SignalId signal_id = OS_SIGNAL_ID_GET(msg_p);
            if (OS_SIG_PULSE == signal_id) {
                SignalSend(OS_SIGNAL_SRC_GET(msg_p), s, OS_SIG_PULSE_ACK);
                goto signal_filter;
            } else if (OS_SIG_PWR == signal_id) {
                const OS_TaskHd thd = OS_TaskGet();
                //OS_SchedulerSuspend();
                if (OS_NULL != thd) {
                    const OS_PowerState state =
                        (OS_PowerState)BF_GET(OS_SIGNAL_DATA_GET(msg_p), 0, BIT_MASK(BIT_SIZE(OS_PowerState)));
                    IF_STATUS_OK(s = OS_TaskPowerStateSet(thd, state)) {
                        const OS_TaskId src_tid = (OS_TaskId)OS_SIGNAL_SRC_GET(msg_p);
                        SignalSend(src_tid, s, OS_SIG_PWR_ACK);
                        goto signal_filter;
                    } else {
                        OS_LOG(D_WARNING, "Power state set failed!");
                    }
                } else { OS_LOG_S(D_WARNING, S_INVALID_REF); }
                //OS_SchedulerResume();
            }
        }
    } else {
//        OS_LOG_S(D_DEBUG, s);
    }
    return s;
}

/******************************************************************************/
void SignalSend(const OS_TaskId src_tid, const Status status, const OS_SignalId signal_id)
{
    if (0 != src_tid) {
        const OS_TaskHd src_thd = OS_TaskByIdGet(src_tid);
        const OS_QueueHd src_stdin_qhd = OS_TaskStdInGet(src_thd);
        if (OS_NULL != src_stdin_qhd) {
            const OS_Signal signal = OS_SIGNAL_CREATE(signal_id, (OS_SignalData)status);
            OS_SIGNAL_SEND(src_stdin_qhd, signal, OS_MSG_PRIO_NORMAL);
        }
    }
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_MessageSend(const OS_QueueHd qhd, const OS_Message* msg_p, const OS_MessagePrio priority)
{
    return OS_ISR_QueueSend(qhd, &msg_p, priority);
}

/******************************************************************************/
Status OS_ISR_MessageReceive(const OS_QueueHd qhd, OS_Message** msg_pp)
{
    return OS_ISR_QueueReceive(qhd, msg_pp);
}