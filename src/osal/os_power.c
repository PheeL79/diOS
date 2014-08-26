/***************************************************************************//**
* @file    os_power.c
* @brief   OS Power.
* @author  A. Filyanov
*******************************************************************************/
#include "os_supervise.h"
#include "os_debug.h"
#include "os_signal.h"
#include "os_message.h"
#include "os_power.h"

//------------------------------------------------------------------------------
extern volatile OS_QueueHd sv_stdin_qhd;

/******************************************************************************/
Status OS_PowerInit(void)
{
    return S_OK;
}

/******************************************************************************/
Status OS_PowerStateSet(const OS_PowerState state)
{
const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_PWR, state);
    return OS_SIGNAL_SEND(sv_stdin_qhd, signal, OS_MSG_PRIO_HIGH);
}

/******************************************************************************/
Status OS_ISR_PowerStateSet(const OS_PowerState state)
{
const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_PWR, state);
    return OS_ISR_SIGNAL_SEND(sv_stdin_qhd, signal, OS_MSG_PRIO_HIGH);
}

/******************************************************************************/
ConstStrPtr OS_PowerStateNameGet(const OS_PowerState state)
{
static ConstStr power_name_str[PWR_LAST][12] = {
    {"undef"},
    {"startup"},
    {"on"},
    {"off"},
    {"sleep"},
    {"stop"},
    {"standby"},
    {"hibernate"},
    {"shutdown"},
    {"battery"},
};
    return &power_name_str[state][0];
}