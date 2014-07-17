#ifndef _OS_SIGNAL_H_
#define _OS_SIGNAL_H_

#include "os_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
typedef U32         OS_Signal;
typedef U8          OS_SignalId;
typedef U16         OS_SignalData;

#define OS_SIGNAL_TOKEN         BIT(BIT_SIZE(OS_Signal) - 1)
#define OS_SIGNAL_ID_MASK       BIT_MASK(BIT_SIZE(OS_SignalId) - 1)
#define OS_SIGNAL_SRC_MASK      BIT_MASK(BIT_SIZE(OS_TaskId))
#define OS_SIGNAL_DATA_MASK     BIT_MASK(BIT_SIZE(OS_SignalData))

enum {
    OS_SIG_UNDEF,
    OS_SIG_SYS = 1,
    OS_SIG_ISR,                             // src = dev ID(drv.h), data - device dependent;
    OS_SIG_STDIN,
    OS_SIG_STDOUT,
    OS_SIG_REBOOT,
    OS_SIG_SHUTDOWN,
    OS_SIG_PWR = 16,                        // data == OS_PowerState;
    OS_SIG_PWR_ACK,                         // data == Status;
    OS_SIG_PWR_BATTERY_CONNECTED,
    OS_SIG_PWR_BATTERY_DISCONNECTED,
    OS_SIG_PWR_BATTERY_CHARGED,
    OS_SIG_PWR_BATTERY_LEVEL,               // data == Charge level;
    OS_SIG_PULSE,
    OS_SIG_PULSE_ACK,
    OS_SIG_TIMER,                           // data == OS_TimerId;
    OS_SIG_EVENT,                           // data == OS_TimerId;
    OS_SIG_APP = 32,                        // app dependent
    OS_SIG_LAST = OS_SIGNAL_ID_MASK
};

//------------------------------------------------------------------------------
#define OS_SIGNAL_CREATE(id, data)                              (OS_SIGNAL_TOKEN | \
                                                                ((((OS_Signal)(id) & OS_SIGNAL_ID_MASK) << 24) | \
                                                                (((OS_Signal)(OS_TaskIdGet(OS_THIS_TASK)) & OS_SIGNAL_SRC_MASK) << 16) | \
                                                                ((OS_Signal)(data) & OS_SIGNAL_DATA_MASK)))
#define OS_SIGNAL_EMIT(qhd, signal, prio)                       OS_MessageSend(qhd, (OS_Message *)signal, OS_NO_BLOCK, prio)
#define OS_SIGNAL_MULTICAST_EMIT(receivers_qhd_v, signal, prio) OS_MessageMulticastSend(receivers_qhd_v, (OS_Message *)signal, OS_NO_BLOCK, prio)
#define OS_SIGNAL_IS(msg)                                       (OS_SIGNAL_TOKEN == ((OS_Signal)(msg) & OS_SIGNAL_TOKEN))
#define OS_SIGNAL_ID_GET(signal)                                ((OS_SignalId)(((OS_Signal)(signal) >> 24) & OS_SIGNAL_ID_MASK))
#define OS_SIGNAL_SRC_GET(signal)                               ((OS_TaskId)(((OS_Signal)(signal) >> 16) & OS_SIGNAL_SRC_MASK))
#define OS_SIGNAL_DATA_GET(signal)                              ((OS_SignalData)(((OS_Signal)(signal)) & OS_SIGNAL_DATA_MASK))

#define OS_ISR_SIGNAL_CREATE(id, data)                          (OS_SIGNAL_TOKEN | \
                                                                ((((OS_Signal)(id) & OS_SIGNAL_ID_MASK) << 24) | \
                                                                (((OS_Signal)(0) & OS_SIGNAL_SRC_MASK) << 16) | \
                                                                ((OS_Signal)(data) & OS_SIGNAL_DATA_MASK)))
#define OS_ISR_SIGNAL_EMIT(qhd, signal, prio)                   OS_ISR_MessageSend(qhd, (OS_Message *)signal, prio)

#ifdef __cplusplus
}
#endif

#endif // _OS_SIGNAL_H_
