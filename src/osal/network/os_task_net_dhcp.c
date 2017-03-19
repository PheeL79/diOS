/***************************************************************************//**
* @file    os_task_net_dhcp.c
* @brief   Network DHCP daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include "lwip/dhcp.h"
#include "os_supervise.h"
#include "os_debug.h"
#include "os_signal.h"
#include "os_timer.h"
#include "os_settings.h"
#include "os_environment.h"
#include "os_network.h"
#include "os_task_net_dhcp.h"

#if (OS_NETWORK_ENABLED) && (OS_NETWORK_DHCP)
//-----------------------------------------------------------------------------
#define MDL_NAME                        "net_dhcp_d"

#define OS_TIM_DHCP_CLIENT_PERIOD_MS    DHCP_FINE_TIMER_MSECS
#define OS_TIM_DHCP_CLIENT_TIMEOUT_MS   DHCP_COARSE_TIMER_MSECS

enum {
    OS_TIM_ID_ETH0_DHCP_CLIENT = OS_TIM_ID_APP
};

//-----------------------------------------------------------------------------
extern Status OS_NetworkItfAddress4Log(const OS_NetworkItfHd net_itf_hd);

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_NetworkItfHd     net_itf_hd;
    OS_TimerHd          dhcp_cli_timer_hd;
    OS_NetworkIpAddr4   ip_addr4;
    OS_NetworkNetMask4  netmask4;
    OS_NetworkGateWay4  gateway4;
} TaskStorage;

//-----------------------------------------------------------------------------
static Status OS_SignalTimerNetDhcpTimeout(OS_Message* const msg_p, TaskStorage* const tstor_p);

//------------------------------------------------------------------------------
const OS_TaskConfig task_net_dhcp_cfg = {
    .name           = OS_DAEMON_NAME_NET_DHCP,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 1,
    .prio_init      = OS_PRIO_TASK_NET_DHCP,
    .prio_power     = OS_PRIO_PWR_TASK_NET_DHCP,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN,
    .stdin_len      = OS_STDIN_LEN,
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_OK;
    OS_LOG(L_INFO, "Init");
    tstor_p->net_itf_hd = ((OS_TaskNetDhcpInitArgs*)args_p->args_p)->net_itf_hd;
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
Status s = S_UNDEF;

	for(;;) {
        IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            OS_LOG_S(L_WARNING, S_INVALID_MESSAGE);
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_TIMER:
                        {
                        const OS_TimerId timer_id = OS_TimerIdGet(tstor_p->dhcp_cli_timer_hd);
                            if (timer_id == OS_SIGNAL_TIMER_ID_GET(msg_p)) {
                                IF_STATUS(s = OS_SignalTimerNetDhcpTimeout(msg_p, tstor_p)) {}
                            }
                        }
                        break;
                    default:
                        OS_LOG_S(L_DEBUG_1, S_INVALID_SIGNAL);
                        break;
                }
            } else {
                switch (msg_p->id) {
                    default:
                        OS_LOG_S(L_DEBUG_1, S_INVALID_MESSAGE);
                        break;
                }
                OS_MessageDelete(msg_p); // free message allocated memory
            }
        }
    }
}

/******************************************************************************/
Status OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state)
{
TaskStorage* const tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_OK;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        case PWR_ON:
            IF_OK(s = OS_NetworkItfAddress4Get(tstor_p->net_itf_hd,
                                               &tstor_p->ip_addr4,
                                               &tstor_p->netmask4,
                                               &tstor_p->gateway4)) {
                if (0 == tstor_p->ip_addr4.addr) {
                    IF_OK(s = OS_NetworkItfDhcpClientStart(tstor_p->net_itf_hd)) {
                        OS_LOG(L_DEBUG_1, "%s: DHCP client setup run", OS_NetworkItfNameGet(tstor_p->net_itf_hd));
                        static ConstStrP tim_name_p = "DHCP cli";
                        const OS_TimerConfig tim_cfg = {
                            .name_p = tim_name_p,
                            .slot   = OS_TaskStdInGet(OS_TaskByNameGet(OS_DAEMON_NAME_NET_DHCP)),
                            .id     = OS_TIM_ID_ETH0_DHCP_CLIENT,
                            .period = OS_TIM_DHCP_CLIENT_PERIOD_MS,
                            .options= (OS_TimerOptions)BIT(OS_TIM_OPT_PERIODIC)
                        };
                        IF_OK(s = OS_TimerCreate(&tim_cfg, &tstor_p->dhcp_cli_timer_hd)) {
                            IF_OK(s = OS_TimerStart(tstor_p->dhcp_cli_timer_hd, OS_TIM_DHCP_CLIENT_TIMEOUT_MS)) {
                            } else { OS_LOG_S(L_WARNING, s); }
                        } else { OS_LOG_S(L_WARNING, s); }
                    }
                }
            }
            break;
        case PWR_STOP:
            break;
        case PWR_SHUTDOWN:
            break;
        default:
            break;
    }
    return s;
}

/******************************************************************************/
Status OS_SignalTimerNetDhcpTimeout(OS_Message* const msg_p, TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
    IF_OK(s = OS_NetworkItfAddress4Get(tstor_p->net_itf_hd,
                                       &tstor_p->ip_addr4,
                                       &tstor_p->netmask4,
                                       &tstor_p->gateway4)) {
        if (tstor_p->ip_addr4.addr) {
            OS_LOG(L_DEBUG_1, "%s: DHCP client setup done", OS_NetworkItfNameGet(tstor_p->net_itf_hd));
            IF_OK(s = OS_TimerDelete(tstor_p->dhcp_cli_timer_hd, OS_TIM_DHCP_CLIENT_TIMEOUT_MS)) {
                IF_OK(s = OS_NetworkItfAddress4Log(tstor_p->net_itf_hd)) {
                    IF_STATUS(s = OS_TaskDelete(OS_THIS_TASK)) {}
                }
            }
        } else { OS_LOG(L_DEBUG_1, "%s: DHCP client timeout!", OS_NetworkItfNameGet(tstor_p->net_itf_hd)); }
    }
    return s;
}

#endif //(OS_NETWORK_ENABLED) && (OS_NETWORK_DHCP)