/***************************************************************************//**
* @file    os_mailbox.c
* @brief   OS Mailbox.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "os_common.h"
#include "os_debug.h"
#include "os_task.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_mailbox.h"

//------------------------------------------------------------------------------
static void SignalSend(const OS_TaskId src_tid, const Status status, const OS_SignalId signal_id);

/******************************************************************************/
OS_Message* OS_MessageCreate(const OS_MessageId id, const OS_MessageData data_p, const OS_MessageSize size, const OS_TimeMs timeout)
{
OS_Message* msg_p = OS_Malloc(size + sizeof(OS_Message));

    if (OS_NULL != msg_p) {
        OS_MemCpy(msg_p->data, data_p, size);
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
Status OS_MessageSend(const OS_QueueHd qhd, const OS_Message* msg_p,
                      const OS_TimeMs timeout, const OS_MessagePrio priority)
{
    return OS_QueueSend(qhd, &msg_p, timeout, priority);
}

/******************************************************************************/
INLINE Status OS_MessageMulticastSend(const OS_List* slots_qhd_l_p, OS_Message* msg_p,
                                      const OS_TimeMs timeout, const OS_MessagePrio priority)
{
OS_Message* msg_inst_p = OS_NULL;
Status s = S_OK;
    if (OS_NULL != msg_p) {
        if (OS_NULL != slots_qhd_l_p) {
            OS_ListItem* iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(slots_qhd_l_p));
            while (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
                const OS_QueueHd slot_qhd = (OS_QueueHd)OS_ListItemValueGet(iter_li_p);
                iter_li_p = OS_ListItemNextGet(iter_li_p);
                if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) { // if the next item is present...
                    if (OS_TRUE != OS_SignalIs(msg_p)) { // ...and it isn't a signal...
                        const Size msg_size = sizeof(OS_Message) + msg_p->size;
                        msg_inst_p = OS_Malloc(msg_size); // ...make an instance of the message.
                        OS_MemCpy(msg_inst_p, msg_p, msg_size);
                    }
                }
                IF_STATUS(s = OS_QueueSend(slot_qhd, &msg_p, timeout, priority)) {
                    if (OS_NULL != msg_inst_p) {
                        OS_MessageDelete(msg_inst_p);
                    }
                    return s;
                }
                if (OS_NULL != msg_inst_p) {
                    msg_p = msg_inst_p;
                }
            }
        } else { s = S_INVALID_PTR; }
    } else { s = S_INVALID_PTR; }
    return s;
}

/******************************************************************************/
Status OS_MessageEmit(OS_Message* msg_p, const OS_TimeMs timeout, const OS_MessagePrio priority)
{
extern const OS_List* OS_TaskSlotsGet(const OS_TaskHd thd);
const OS_List* slots_qhd_l_p = OS_TaskSlotsGet(OS_THIS_TASK);
    return OS_MessageMulticastSend(slots_qhd_l_p, msg_p, timeout, priority);
}

/******************************************************************************/
Status OS_MessageReceive(const OS_QueueHd qhd, OS_Message** msg_pp, const OS_TimeMs timeout)
{
extern Status OS_TaskPowerStateSet(const OS_TaskHd thd, const OS_PowerState state);
Status s = S_UNDEF;
signal_filter: //Prevent recursion calls.
    IF_OK(s = OS_QueueReceive(qhd, msg_pp, timeout)) {
        const OS_Message* msg_p = *msg_pp;
        if (OS_SignalIs(msg_p)) { //Filter system signals.
            const OS_SignalId signal_id = OS_SignalIdGet(msg_p);
            if (OS_SIG_PULSE == signal_id) {
                SignalSend(OS_SignalSrcGet(msg_p), s, OS_SIG_PULSE_ACK);
                goto signal_filter;
            } else if (OS_SIG_PWR == signal_id) {
                const OS_TaskHd thd = OS_TaskGet();
                //OS_SchedulerSuspend();
                if (OS_NULL != thd) {
                    const OS_PowerState state =
                        (OS_PowerState)BF_GET(OS_SignalDataGet(msg_p), 0, BIT_SIZE(OS_PowerState));
                    IF_OK(s = OS_TaskPowerStateSet(thd, state)) {
                        const OS_TaskId src_tid = (OS_TaskId)OS_SignalSrcGet(msg_p);
                        SignalSend(src_tid, s, OS_SIG_PWR_ACK);
                        goto signal_filter;
                    } else {
                        OS_LOG(L_WARNING, "Power state set failed!");
                    }
                } else { OS_LOG_S(L_WARNING, S_INVALID_PTR); }
                //OS_SchedulerResume();
            }
        }
    } else {
//        OS_LOG_S(L_DEBUG_1, s);
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
            const OS_Signal signal = OS_SignalCreate(signal_id, (OS_SignalData)status);
            OS_SignalSend(src_stdin_qhd, signal, OS_MSG_PRIO_NORMAL);
        }
    }
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
OS_Message* OS_ISR_MessageCreate(const OS_MessageSrc src, const OS_MessageId id, const OS_MessageData data_p, const OS_MessageSize size)
{
OS_Message* msg_p = OS_ISR_Malloc(size + sizeof(OS_Message));

    if (OS_NULL != msg_p) {
        OS_MemCpy(msg_p->data, data_p, size);
        msg_p->id   = id;
        msg_p->size = size;
        msg_p->src  = src;
    }
    return msg_p;
}

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