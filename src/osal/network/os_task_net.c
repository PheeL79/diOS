/***************************************************************************//**
* @file    os_task_net.c
* @brief   Network daemon task.
* @author  A. Filyanov
*******************************************************************************/
#include "lwip/tcpip.h"
#include "os_supervise.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_mailbox.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_timer.h"
#include "os_settings.h"
#include "os_network.h"
#include "os_task_net.h"

#if (OS_NETWORK_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME                    "network_d"

#define OS_TIM_DHCP_CLIENT_PERIOD_MS   100
#define OS_TIM_DHCP_CLIENT_TIMEOUT_MS  1000
enum {
    OS_TIM_ID_ETH0_DHCP_CLIENT = OS_TIM_ID_APP
};

extern Status OS_NetworkItfAddress4Log(const OS_NetworkItfHd net_itf_hd);

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_NetworkItfHd net_itf_hd[OS_NETWORK_ITF_LAST];
#if (OS_NETWORK_DHCP)
    Bool            dhcp_client;
    OS_TimerHd      dhcp_cli_timer_hd;
#endif //(OS_NETWORK_DHCP)
} TaskStorage;

//------------------------------------------------------------------------------
const OS_TaskConfig task_net_cfg = {
    .name           = OS_DAEMON_NAME_NET,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 10,
    .prio_init      = OS_PRIO_TASK_NET,
    .prio_power     = OS_PRIO_PWR_TASK_NET,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN * 2,
    .stdin_len      = 16,
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
#if (OS_NETWORK_TCP)
    /* Create tcp_ip stack thread */
    tcpip_init(OS_NULL, OS_NULL);
#endif //(OS_NETWORK_TCP)
    tstor_p->dhcp_client = (1 == OS_NETWORK_DHCP) ? OS_TRUE : OS_FALSE;
#if (OS_NETWORK_HAVE_LOOPIF)
    {
        const OS_NetworkItfConfig net_itf_cfg = {
            .name       = "loopback",
            .drv_cfg_p  = OS_NULL,
            .net_itf_id = OS_NETWORK_ITF_LO,
            .is_loopback= OS_TRUE
        };
        IF_STATUS(s = OS_NetworkItfCreate(&net_itf_cfg, &tstor_p->net_itf_hd[OS_NETWORK_ITF_LO])) { return s; }
    }
#endif //(OS_NETWORK_HAVE_LOOPIF)
    {
        OS_DriverConfig drv_cfg = {
            .name       = "ETH0",
            .itf_p      = drv_eth_v[DRV_ID_ETH0],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_NetworkItfConfig net_itf_cfg = {
            .name       = "eth0",
            .drv_cfg_p  = &drv_cfg,
            .net_itf_id = OS_NETWORK_ITF_ETH0,
            .is_loopback= OS_FALSE
        };
        IF_STATUS(s = OS_NetworkItfCreate(&net_itf_cfg, &tstor_p->net_itf_hd[OS_NETWORK_ITF_ETH0])) { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
#if (OS_NETWORK_DHCP)
    OS_NetworkIpAddr4  ip_addr4 = { 0 };
    OS_NetworkNetMask4 netmask4 = { 0 };
    OS_NetworkGateWay4 gateway4 = { 0 };
#endif //(OS_NETWORK_DHCP)
Status s = S_UNDEF;

	for(;;) {
        IF_STATUS(s = OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
            if (S_MODULE != s) {
                OS_LOG_S(L_WARNING, S_INVALID_MESSAGE);
            }
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_ETH_RX:
                        {
                        const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(OS_SignalSrcGet(msg_p));
                            OS_NetworkRead(net_itf_hd, OS_NULL, OS_SignalDataGet(msg_p));
                        }
                        break;
                    case OS_SIG_ETH_LINK_STATE_CHANGED:
                        {
                        const OS_NetworkItfId net_itf_id = OS_SignalDataGet(msg_p);
                        const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(net_itf_id);
                            IF_OK(s = OS_NetworkItfLinkStateRefresh(net_itf_hd)) {
                                if (OS_TRUE == OS_NetworkItfLinkStateGet(net_itf_hd)) {
#if (OS_NETWORK_DHCP)
                                    if (OS_TRUE == tstor_p->dhcp_client) {
                                        IF_OK(s = OS_NetworkItfAddress4Get(net_itf_hd, &ip_addr4, &netmask4, &gateway4)) {
                                            if (0 == ip_addr4.addr) {
                                                IF_OK(s = OS_NetworkItfDhcpClientStart(net_itf_hd)) {
                                                    OS_LOG(L_DEBUG_1, "%s: DHCP client setup run", OS_NetworkItfNameGet(net_itf_hd));
                                                    static ConstStrP tim_name_p = "DHCP cli";
                                                    const OS_TimerConfig tim_cfg = {
                                                        .name_p = tim_name_p,
                                                        .slot   = stdin_qhd,
                                                        .id     = OS_TIM_ID_ETH0_DHCP_CLIENT,
                                                        .period = OS_TIM_DHCP_CLIENT_PERIOD_MS,
                                                        .options= (OS_TimerOptions)BIT(OS_TIM_OPT_PERIODIC)
                                                    };
                                                    IF_OK(s = OS_TimerCreate(&tim_cfg, &tstor_p->dhcp_cli_timer_hd)) {
                                                        IF_OK(s = OS_TimerStart(tstor_p->dhcp_cli_timer_hd, OS_TIM_DHCP_CLIENT_TIMEOUT_MS)) {
                                                        } else {
                                                            OS_LOG_S(L_WARNING, s);
                                                        }
                                                    } else {
                                                        OS_LOG_S(L_WARNING, s);
                                                    }
                                                }
                                            }
                                        }
                                    }
#endif //(OS_NETWORK_DHCP)
                                }
                            } else {
                                OS_LOG_S(L_WARNING, s);
                            }
                        }
                        break;
                    case OS_SIG_TIMER:
#if (OS_NETWORK_DHCP)
                        if (OS_TRUE == tstor_p->dhcp_client) {
                            const OS_TimerId timer_id = OS_TimerIdGet(tstor_p->dhcp_cli_timer_hd);
                            if (timer_id == OS_SIGNAL_TIMER_ID_GET(msg_p)) {
                                const OS_NetworkItfHd net_itf_hd = tstor_p->net_itf_hd[OS_NETWORK_ITF_ETH0];
                                IF_OK(s = OS_NetworkItfAddress4Get(net_itf_hd, &ip_addr4, &netmask4, &gateway4)) {
                                    if (0 != ip_addr4.addr) {
                                        IF_OK(s = OS_NetworkItfDhcpClientStop(net_itf_hd)) {
                                            OS_LOG(L_DEBUG_1, "%s: DHCP client setup done", OS_NetworkItfNameGet(net_itf_hd));
                                            IF_OK(s = OS_TimerDelete(tstor_p->dhcp_cli_timer_hd, OS_TIM_DHCP_CLIENT_TIMEOUT_MS)) {
                                                IF_OK(s = OS_NetworkItfAddress4Log(net_itf_hd)) {
                                                }
                                            }
                                        }
                                    } else {
                                        OS_LOG(L_DEBUG_1, "%s: DHCP client timeout!", OS_NetworkItfNameGet(net_itf_hd));
                                    }
                                }
                            }
                        }
#endif //(OS_NETWORK_DHCP)
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
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {
            }
            break;
        case PWR_ON:
            {
            OS_NetworkIpAddr4  ip_addr4         = { 0 };
            OS_NetworkNetMask4 netmask4         = { 0 };
            OS_NetworkGateWay4 gateway4         = { 0 };
            U8 mac_addr[HAL_ETH_MAC_ADDR_SIZE]  = { 0 };
            OS_NetworkItfInitArgs net_itf_init_args = {
                .host_name_p    = OS_NULL,
                .mac_addr_p     = (OS_NetworkMacAddr*)&mac_addr,
                .ip_addr4_p     = &ip_addr4,
                .netmask4_p     = &netmask4,
                .gateway4_p     = &gateway4
            };
            OS_NetworkItfHd net_itf_hd = OS_NULL;
#if (OS_NETWORK_HAVE_LOOPIF)
            OS_NETWORK_IP4_ADDR(&ip_addr4, 127, 0, 0, 1);
            OS_NETWORK_IP4_ADDR(&netmask4, 255, 0, 0, 0);
            OS_NETWORK_IP4_ADDR(&gateway4, 127, 0, 0, 1);
            net_itf_hd = tstor_p->net_itf_hd[OS_NETWORK_ITF_LO];
            net_itf_init_args.host_name_p = (StrP)OS_Malloc(OS_NETWORK_ITF_HOST_NAME_LEN);
            if (net_itf_init_args.host_name_p) {
                ConstStrP localhost_sp = "localhost";
                if (OS_NETWORK_ITF_HOST_NAME_LEN >= (OS_StrLen(localhost_sp) + 1)) {
                    OS_StrCpy(net_itf_init_args.host_name_p, localhost_sp);
                } else {
                    s = S_INVALID_SIZE;
                    goto error;
                }
            }
            IF_STATUS(s = OS_NetworkItfInit(net_itf_hd, &net_itf_init_args)) { goto error; }
            const OS_NetworkItfOpenArgs net_itf_open_args = {
                .netd_stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK)
            };
            IF_STATUS(s = OS_NetworkItfOpen(net_itf_hd, &net_itf_open_args)) { goto error; }
            IF_STATUS(s = OS_NetworkItfLinkUpSet(net_itf_hd)) { goto error; }
            IF_STATUS(s = OS_NetworkItfUpSet(net_itf_hd)) { goto error; }
#endif //(OS_NETWORK_HAVE_LOOPIF)

            OS_NETWORK_IP4_ADDR(&ip_addr4, 0, 0, 0, 0);
            OS_NETWORK_IP4_ADDR(&netmask4, 0, 0, 0, 0);
            OS_NETWORK_IP4_ADDR(&gateway4, 0, 0, 0, 0);
            net_itf_hd = tstor_p->net_itf_hd[OS_NETWORK_ITF_ETH0];
            net_itf_init_args.host_name_p = (StrP)OS_Malloc(OS_NETWORK_ITF_HOST_NAME_LEN);
#if (OS_SETTINGS_ENABLED)
            ConstStrP config_path_p = OS_EnvVariableGet("config_file");
            Str value[OS_SETTINGS_VALUE_LEN];
            Str net_itf_sect_str[0x10] = "Network\\";
            if (net_itf_sect_str != OS_StrCat(net_itf_sect_str, OS_NetworkItfNameGet(net_itf_hd))) {
                s = S_INVALID_PTR;
                goto error;
            }
#endif //(OS_SETTINGS_ENABLED)
            {
#if (OS_SETTINGS_ENABLED)
                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "mac_addr", &value[0])) { goto error; }
                IF_STATUS(s = OS_NetworkMacAddressStrToBin(value, mac_addr)) { goto error; }
#else
                IF_STATUS(s = OS_NetworkMacAddressStrToBin(value, OS_NETWORK_MAC_ADDR_DEFAULT)) { goto error; }
#endif //(OS_SETTINGS_ENABLED)
#if (OS_SETTINGS_ENABLED)
                if (net_itf_init_args.host_name_p) {
                    IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "host_name", &value[0])) { goto error; }
                    if (OS_NETWORK_ITF_HOST_NAME_LEN >= (OS_StrLen(&value[0]) + 1)) {
                        OS_StrCpy(net_itf_init_args.host_name_p, &value[0]);
                    } else {
                        s = S_INVALID_SIZE;
                        goto error;
                    }
                }
#endif //(OS_SETTINGS_ENABLED)
#if (OS_NETWORK_DHCP)
#   if (OS_SETTINGS_ENABLED)
                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "dhcp_client", &value[0])) { goto error; }
                tstor_p->dhcp_client = (0 == OS_StrCmp(&value[0], OS_TRUE_STR)) ? OS_TRUE : OS_FALSE;
#   endif //(OS_SETTINGS_ENABLED)
                if (OS_TRUE != tstor_p->dhcp_client) {
#endif //(OS_NETWORK_DHCP)
#if (OS_SETTINGS_ENABLED)
                    IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "ip_addr4", &value[0])) { goto error; }
                    IF_STATUS(s = OS_NetworkIpAddress4StrToBin(&value[0], net_itf_init_args.ip_addr4_p)) { goto error; }
                    IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "netmask4", &value[0])) { goto error; }
                    IF_STATUS(s = OS_NetworkNetMask4StrToBin(&value[0], net_itf_init_args.netmask4_p)) { goto error; }
                    IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "gateway4", &value[0])) { goto error; }
                    IF_STATUS(s = OS_NetworkGateWay4StrToBin(&value[0], net_itf_init_args.gateway4_p)) { goto error; }
#else
                    if (hostname_p) {
                        OS_StrCpy(hostname_p, OS_NETWORK_HOST_NAME);
                    }
                    IF_STATUS(s = OS_NetworkIpAddress4StrToBin(OS_NETWORK_IP_ADDR4_DEFAULT, net_itf_init_args.ip_addr4_p)) { goto error; }
                    IF_STATUS(s = OS_NetworkNetMask4StrToBin(OS_NETWORK_NETMASK4_DEFAULT, net_itf_init_args.netmask4_p)) { goto error; }
                    IF_STATUS(s = OS_NetworkGateWay4StrToBin(OS_NETWORK_GATEWAY4_DEFAULT, net_itf_init_args.gateway4_p)) { goto error; }
#endif //(OS_SETTINGS_ENABLED)
#if (OS_NETWORK_DHCP)
                }
#endif //(OS_NETWORK_DHCP)
                IF_STATUS(s = OS_NetworkItfInit(net_itf_hd, &net_itf_init_args)) { goto error; }
                const OS_DriverHd drv_gpio_dhd = OS_DriverGpioGet();
                const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_TaskByNameGet(task_net_cfg.name));
                const DrvGpioArgsIoCtlOpen args = {
                    .gpio       = GPIO_ETH_MDINT,
                    .signal_id  = OS_SIG_ETH_LINK_STATE_CHANGED,
                    .signal_data= OS_NETWORK_ITF_ETH0,
                    .slot_qhd   = stdin_qhd
                };
                IF_STATUS(s = OS_DriverIoCtl(drv_gpio_dhd, DRV_REQ_GPIO_OPEN, (void*)&args)) { goto error; }
            }
            IF_OK(s = OS_NetworkItfAddress4Log(net_itf_hd)) {
            }
            {
                const OS_NetworkItfOpenArgs net_itf_open_args = {
                    .netd_stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK)
                };
                IF_STATUS(s = OS_NetworkItfOpen(net_itf_hd, &net_itf_open_args)) { goto error; }
            }
            }
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN: {
            OS_NetworkItfHd net_itf_hd = OS_NULL;
#if (OS_NETWORK_HAVE_LOOPIF)
                net_itf_hd = tstor_p->net_itf_hd[OS_NETWORK_ITF_LO];
                IF_OK(s = OS_NetworkItfClose(net_itf_hd, OS_NULL)) {
                    IF_OK(s = OS_NetworkItfDeInit(net_itf_hd, OS_NULL)) {
#endif //(OS_NETWORK_HAVE_LOOPIF)
                        net_itf_hd = tstor_p->net_itf_hd[OS_NETWORK_ITF_ETH0];
                        IF_OK(s = OS_NetworkItfClose(net_itf_hd, OS_NULL)) {
                            IF_OK(s = OS_NetworkItfDeInit(net_itf_hd, OS_NULL)) {
                                const OS_DriverHd drv_gpio_dhd = OS_DriverGpioGet();
                                IF_OK(s = OS_DriverIoCtl(drv_gpio_dhd, DRV_REQ_GPIO_CLOSE, (void*)GPIO_ETH_MDINT)) {
                                    IF_OK(s = OS_QueueClear(OS_TaskStdInGet(OS_THIS_TASK))) {
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
error:
    IF_STATUS(s) { OS_LOG_S(L_WARNING, s); }
    return s;
}

#endif //(OS_NETWORK_ENABLED)
