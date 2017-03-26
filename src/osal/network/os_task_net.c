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
#if (OS_NETWORK_DHCP)
#include "os_task_net_dhcp.h"
#endif //(OS_NETWORK_DHCP)

#if (OS_NETWORK_ENABLED)
//------------------------------------------------------------------------------
#define MDL_NAME                        "network_d"

//------------------------------------------------------------------------------
extern Status OS_NetworkItfAddress4Log(const OS_NetworkItfHd net_itf_hd);

//------------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_QueueHd      netd_qhd;
    OS_QueueSetHd   netd_qshd;
    OS_NetworkItfHd net_itf_hd_v[OS_NETWORK_ITF_LAST];
    Bool            dhcp_client;
#if (OS_NETWORK_PERF)
    void*           iperf_sess_p;
#endif //(OS_NETWORK_PERF)
} TaskStorage;

//------------------------------------------------------------------------------
const OS_TaskConfig task_net_cfg = {
    .name           = OS_DAEMON_NAME_NET,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = 0,
    .timeout        = 10,
    .prio_init      = OS_PRIO_TASK_NET,
    .prio_power     = OS_PRIO_PWR_TASK_NET,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN,
    .stdin_len      = OS_STDIN_LEN,
};

//------------------------------------------------------------------------------
static Status OS_TaskPowerOn(TaskStorage* const tstor_p);
static Status OS_TaskPowerShutdown(TaskStorage* const tstor_p);

static Status NetworkServicesStart(TaskStorage* const tstor_p);
static Status NetworkServicesStop(TaskStorage* const tstor_p);

static Status OS_SignalNetEthLinkStateChangedHandler(OS_Message* const msg_p, TaskStorage* const tstor_p);

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
static const OS_QueueConfig que_cfg = {
    .len        = OS_NETWORK_DAEMON_QUEUE_LEN,
    .item_size  = sizeof(OS_Message*)
};
    IF_STATUS(s = OS_QueueCreate(&que_cfg, OS_TaskGet(), &tstor_p->netd_qhd)) { return s; }
    IF_STATUS(s = OS_QueueSetCreate((task_net_cfg.stdin_len + que_cfg.len), &tstor_p->netd_qshd)) { return s; }
    IF_STATUS(s = OS_QueueSetItemAppend(tstor_p->netd_qshd, OS_TaskStdInGet(OS_THIS_TASK))) { return s; }
    IF_STATUS(s = OS_QueueSetItemAppend(tstor_p->netd_qshd, tstor_p->netd_qhd)) { return s; }
#if (OS_NETWORK_TCP)
    /* Create tcp_ip stack thread */
    OS_LOG(L_DEBUG_1, "TCPIP init");
    tcpip_init(OS_NULL, OS_NULL);
#endif //(OS_NETWORK_TCP)
#if (OS_NETWORK_HAVE_LOOPIF)
    {
        const OS_NetworkItfConfig net_itf_cfg = {
            .name       = "loopback",
            .drv_cfg_p  = OS_NULL,
            .net_itf_id = OS_NETWORK_ITF_LO,
            .is_loopback= OS_TRUE
        };
        IF_STATUS(s = OS_NetworkItfCreate(&net_itf_cfg, &tstor_p->net_itf_hd_v[OS_NETWORK_ITF_LO])) { return s; }
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
        IF_STATUS(s = OS_NetworkItfCreate(&net_itf_cfg, &tstor_p->net_itf_hd_v[OS_NETWORK_ITF_ETH0])) { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
OS_QueueSetItemHd queue_set_item_hd;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
Status s = S_UNDEF;

	for(;;) {
        if (tstor_p->netd_qshd) {
            queue_set_item_hd = OS_QueueSetReceive(tstor_p->netd_qshd, OS_BLOCK);
            if (stdin_qhd == queue_set_item_hd) {
                IF_OK(s = OS_MessageReceive(queue_set_item_hd, &msg_p, OS_NO_BLOCK)) {
                    if (OS_SignalIs(msg_p)) {
                    } else {
                        OS_MessageDelete(msg_p); // free message allocated memory
                    }
                }
            } else if (tstor_p->netd_qhd == queue_set_item_hd) {
                IF_OK(s = OS_MessageReceive(queue_set_item_hd, &msg_p, OS_NO_BLOCK)) {
                    if (OS_SignalIs(msg_p)) {
                        switch (OS_SignalIdGet(msg_p)) {
                            case OS_SIG_NET_ETH_RX: {
                                const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(OS_SignalSrcGet(msg_p));
                                    OS_NetworkRead(net_itf_hd, OS_NULL, OS_SignalDataGet(msg_p));
                                }
                                break;
                            case OS_SIG_NET_ETH_LINK_STATE_CHANGED:
                                IF_STATUS(s = OS_SignalNetEthLinkStateChangedHandler(msg_p, tstor_p)) {}
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
                } else { OS_LOG_S(L_WARNING, s); }
            } else { s = S_INVALID_QUEUE; OS_LOG_S(L_WARNING, s); }
        } else {
            IF_OK(s = OS_MessageReceive(stdin_qhd, &msg_p, OS_NO_BLOCK)) {
                if (OS_SignalIs(msg_p)) {
                } else {
                    OS_MessageDelete(msg_p); // free message allocated memory
                }
            }
        }
    }
}

/******************************************************************************/
Status OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state)
{
TaskStorage* const tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
    switch (state) {
        case PWR_STARTUP:
            IF_STATUS(s = OS_TaskInit(args_p)) {}
            break;
        case PWR_ON:
            IF_STATUS(s = OS_TaskPowerOn(tstor_p)) {}
            break;
        case PWR_STOP:
        case PWR_SHUTDOWN:
            IF_STATUS(s = OS_TaskPowerShutdown(tstor_p)) {}
            break;
        default:
            break;
    }

    IF_STATUS(s) { OS_LOG_S(L_WARNING, s); }
    return s;
}

/******************************************************************************/
Status OS_TaskPowerOn(TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
#if (OS_NETWORK_IP_V4)
    OS_NetworkIpAddr4  ip_addr4         = { 0 };
    OS_NetworkNetMask4 netmask4         = { 0 };
    OS_NetworkGateWay4 gateway4         = { 0 };
#endif //(OS_NETWORK_IP_V4)
#if (OS_NETWORK_IP_V6)
    OS_NetworkIpAddr6  ip_addr6         = { 0 };
//    OS_NetworkNetMask6 netmask6         = { 0 };
//    OS_NetworkGateWay6 gateway6         = { 0 };
#endif //(OS_NETWORK_IP_V6)
    U8 mac_addr[HAL_ETH_MAC_ADDR_SIZE]  = { 0 };
    OS_NetworkItfInitArgs net_itf_init_args = {
        .host_name_p= OS_NULL,
        .mac_addr_p = (OS_NetworkMacAddr*)&mac_addr,
#if (OS_NETWORK_IP_V4)
        .ip_addr4_p = &ip_addr4,
        .netmask4_p = &netmask4,
        .gateway4_p = &gateway4,
#endif //(OS_NETWORK_IP_V4)
#if (OS_NETWORK_IP_V6)
        .ip_addr6_p = &ip_addr6,
//        .netmask6_p = &netmask6,
//        .gateway6_p = &gateway6,
#endif //(OS_NETWORK_IP_V6)
    };
    OS_NetworkItfHd net_itf_hd = OS_NULL;
#if (OS_NETWORK_HAVE_LOOPIF)
    OS_NETWORK_IP4_ADDR(&ip_addr4, 127, 0, 0, 1);
    OS_NETWORK_IP4_ADDR(&netmask4, 255, 0, 0, 0);
    OS_NETWORK_IP4_ADDR(&gateway4, 127, 0, 0, 1);
    net_itf_hd = tstor_p->net_itf_hd_v[OS_NETWORK_ITF_LO];
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
        .netd_stdin_qhd = tstor_p->netd_qhd
    };
    IF_STATUS(s = OS_NetworkItfOpen(net_itf_hd, &net_itf_open_args)) { goto error; }
    IF_STATUS(s = OS_NetworkItfLinkUpSet(net_itf_hd)) { goto error; }
    IF_STATUS(s = OS_NetworkItfUpSet(net_itf_hd)) { goto error; }
#endif //(OS_NETWORK_HAVE_LOOPIF)

    OS_NETWORK_IP4_ADDR(&ip_addr4, 0, 0, 0, 0);
    OS_NETWORK_IP4_ADDR(&netmask4, 0, 0, 0, 0);
    OS_NETWORK_IP4_ADDR(&gateway4, 0, 0, 0, 0);
    net_itf_hd = tstor_p->net_itf_hd_v[OS_NETWORK_ITF_ETH0];
    net_itf_init_args.host_name_p = (StrP)OS_Malloc(OS_NETWORK_ITF_HOST_NAME_LEN);
#if (OS_SETTINGS_ENABLED)
    ConstStrP config_path_p = OS_EnvVariableGet("config_file");
    Str value[OS_SETTINGS_VALUE_LEN];
    Str net_itf_sect_str[0x10] = OS_NETWORK_SETTINGS_SECT;
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
        if (net_itf_init_args.host_name_p) {
#if (OS_SETTINGS_ENABLED)
            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "host_name", &value[0])) { goto error; }
            if (OS_NETWORK_ITF_HOST_NAME_LEN >= (OS_StrLen(&value[0]) + 1)) {
                OS_StrCpy(net_itf_init_args.host_name_p, &value[0]);
            } else {
                s = S_INVALID_SIZE;
                goto error;
            }
#else
            if (hostname_p) {
                OS_StrCpy(hostname_p, OS_NETWORK_HOST_NAME);
            }
#endif //(OS_SETTINGS_ENABLED)
        }

#if (OS_NETWORK_DHCP)
#   if (OS_SETTINGS_ENABLED)
        IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "dhcp_client", &value[0])) { goto error; }
        tstor_p->dhcp_client = (0 == OS_StrCmp(&value[0], OS_TRUE_STR)) ? OS_TRUE : OS_FALSE;
#   endif //(OS_SETTINGS_ENABLED)
        if (OS_TRUE != tstor_p->dhcp_client) {
#endif //(OS_NETWORK_DHCP)
#if (OS_NETWORK_IP_V4)
#   if (OS_SETTINGS_ENABLED)
            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "ip_addr4", &value[0])) { goto error; }
            IF_STATUS(s = OS_NetworkIpAddress4StrToBin(&value[0], net_itf_init_args.ip_addr4_p)) { goto error; }
            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "netmask4", &value[0])) { goto error; }
            IF_STATUS(s = OS_NetworkNetMask4StrToBin(&value[0], net_itf_init_args.netmask4_p)) { goto error; }
            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "gateway4", &value[0])) { goto error; }
            IF_STATUS(s = OS_NetworkGateWay4StrToBin(&value[0], net_itf_init_args.gateway4_p)) { goto error; }
#   else
            IF_STATUS(s = OS_NetworkIpAddress4StrToBin(OS_NETWORK_IP_ADDR4_DEFAULT, net_itf_init_args.ip_addr4_p)) { goto error; }
            IF_STATUS(s = OS_NetworkNetMask4StrToBin(OS_NETWORK_NETMASK4_DEFAULT, net_itf_init_args.netmask4_p)) { goto error; }
            IF_STATUS(s = OS_NetworkGateWay4StrToBin(OS_NETWORK_GATEWAY4_DEFAULT, net_itf_init_args.gateway4_p)) { goto error; }
#   endif //(OS_SETTINGS_ENABLED)
#endif //(OS_NETWORK_IP_V4)

#if (OS_NETWORK_IP_V6)
#   if (OS_SETTINGS_ENABLED)
            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "ip_addr6", &value[0])) { goto error; }
//            IF_STATUS(s = OS_NetworkIpAddress6StrToBin(&value[0], net_itf_init_args.ip_addr6_p)) { goto error; }
//            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "netmask6", &value[0])) { goto error; }
//            IF_STATUS(s = OS_NetworkNetMask6StrToBin(&value[0], net_itf_init_args.netmask6_p)) { goto error; }
//            IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "gateway6", &value[0])) { goto error; }
//            IF_STATUS(s = OS_NetworkGateWay6StrToBin(&value[0], net_itf_init_args.gateway6_p)) { goto error; }
#   else
//            IF_STATUS(s = OS_NetworkIpAddress6StrToBin(OS_NETWORK_IP_ADDR6_DEFAULT, net_itf_init_args.ip_addr6_p)) { goto error; }
//            IF_STATUS(s = OS_NetworkNetMask6StrToBin(OS_NETWORK_NETMASK6_DEFAULT, net_itf_init_args.netmask6_p)) { goto error; }
//            IF_STATUS(s = OS_NetworkGateWay6StrToBin(OS_NETWORK_GATEWAY6_DEFAULT, net_itf_init_args.gateway6_p)) { goto error; }
#   endif //(OS_SETTINGS_ENABLED)
#endif //(OS_NETWORK_IP_V6)
#if (OS_NETWORK_DHCP)
        } //otherwise keep zeroes as network address until DHCP respond.
#endif //(OS_NETWORK_DHCP)

//DEBUG!!!
//IP6_ADDR_PART(net_itf_init_args.ip_addr6_p, 0, 0x20, 0x01, 0x0d, 0xb8);
//IP6_ADDR_PART(net_itf_init_args.ip_addr6_p, 1, 0x85, 0xa3, 0x08, 0xd3);
//IP6_ADDR_PART(net_itf_init_args.ip_addr6_p, 2, 0x13, 0x19, 0x8a, 0x2e);
//IP6_ADDR_PART(net_itf_init_args.ip_addr6_p, 3, 0x03, 0x70, 0x73, 0x48);
//DEBUG!!!
        IF_STATUS(s = OS_NetworkItfInit(net_itf_hd, &net_itf_init_args)) { goto error; }
        const OS_DriverHd drv_gpio_dhd = OS_DriverGpioGet();
        const DrvGpioArgsIoCtlOpen args = {
            .gpio       = GPIO_ETH_MDINT,
            .signal_id  = OS_SIG_NET_ETH_LINK_STATE_CHANGED,
            .signal_data= OS_NETWORK_ITF_ETH0,
            .slot_qhd   = tstor_p->netd_qhd
        };
        IF_STATUS(s = OS_DriverIoCtl(drv_gpio_dhd, DRV_REQ_GPIO_OPEN, (void*)&args)) { goto error; }
    }

    IF_OK(s = OS_NetworkItfAddress4Log(net_itf_hd)) {}

    {
        const OS_NetworkItfOpenArgs net_itf_open_args = {
            .netd_stdin_qhd = tstor_p->netd_qhd
        };
        IF_OK(s = OS_NetworkItfOpen(net_itf_hd, &net_itf_open_args)) {
            IF_STATUS(s = NetworkServicesStart(tstor_p)) {}
        } else { goto error; }
    }

error:
    IF_STATUS(s) { OS_LOG_S(L_WARNING, s); }
    return s;
}

/******************************************************************************/
Status OS_TaskPowerShutdown(TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
OS_NetworkItfHd net_itf_hd = OS_NULL;
#if (OS_NETWORK_HAVE_LOOPIF)
    net_itf_hd = tstor_p->net_itf_hd_v[OS_NETWORK_ITF_LO];
    IF_OK(s = OS_NetworkItfClose(net_itf_hd, OS_NULL)) {
        IF_OK(s = OS_NetworkItfDeInit(net_itf_hd, OS_NULL)) {
#endif //(OS_NETWORK_HAVE_LOOPIF)
            net_itf_hd = tstor_p->net_itf_hd_v[OS_NETWORK_ITF_ETH0];
#if (OS_NETWORK_DHCP)
            IF_STATUS(s = OS_NetworkItfDhcpClientReleaseStop(net_itf_hd)) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_DHCP)
            IF_OK(s = NetworkServicesStop(tstor_p)) {
                IF_OK(s = OS_NetworkItfClose(net_itf_hd, OS_NULL)) {
                    IF_OK(s = OS_NetworkItfDeInit(net_itf_hd, OS_NULL)) {
                        const OS_DriverHd drv_gpio_dhd = OS_DriverGpioGet();
                        IF_OK(s = OS_DriverIoCtl(drv_gpio_dhd, DRV_REQ_GPIO_CLOSE, (void*)GPIO_ETH_MDINT)) {
                            IF_OK(s = OS_QueueSetDelete(tstor_p->netd_qshd)) {
                                IF_OK(s = OS_QueueDelete(tstor_p->netd_qhd)) {
                                }
                            }
                        }
                    }
                }
            }
        }
    }

//error:
    IF_STATUS(s) { OS_LOG_S(L_WARNING, s); }
    return s;
}

/******************************************************************************/
Status NetworkServicesStart(TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
#if (OS_NETWORK_PERF)
    IF_STATUS(s = OS_NetworkIperfStart(&tstor_p->iperf_sess_p)) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_PERF)
#if (OS_NETWORK_SNMP)
    IF_STATUS(s = OS_NetworkSnmpStart()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_SNMP)
#if (OS_NETWORK_SNTP)
    IF_STATUS(s = OS_NetworkSntpStart()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_SNTP)
#if (OS_NETWORK_NETBIOS)
    IF_STATUS(s = OS_NetworkNetBiosStart()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_NETBIOS)
#if (OS_NETWORK_HTTPD)
    IF_STATUS(s = OS_NetworkHttpDStart()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_HTTPD)
    return s;
}

/******************************************************************************/
Status NetworkServicesStop(TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
#if (OS_NETWORK_PERF)
    IF_STATUS(s = OS_NetworkIperfStop(tstor_p->iperf_sess_p)) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_PERF)
#if (OS_NETWORK_HTTPD)
    IF_STATUS(s = OS_NetworkHttpDStop()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_HTTPD)
#if (OS_NETWORK_SNMP)
    IF_STATUS(s = OS_NetworkSnmpStop()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_SNMP)
#if (OS_NETWORK_SNTP)
    IF_STATUS(s = OS_NetworkSntpStop()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_SNTP)
#if (OS_NETWORK_NETBIOS)
    IF_STATUS(s = OS_NetworkNetBiosStop()) { OS_LOG_S(L_WARNING, s); }
#endif //(OS_NETWORK_NETBIOS)
    return s;
}

/******************************************************************************/
Status OS_SignalNetEthLinkStateChangedHandler(OS_Message* const msg_p, TaskStorage* const tstor_p)
{
Status s = S_UNDEF;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
const OS_NetworkItfId net_itf_id = OS_SignalDataGet(msg_p);
const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(net_itf_id);
    IF_OK(s = OS_NetworkItfLinkStateRefresh(net_itf_hd)) {
        if (OS_TRUE == OS_NetworkItfLinkStateGet(net_itf_hd)) {
#if (OS_NETWORK_DHCP)
            if (tstor_p->dhcp_client) {
                extern OS_TaskConfig task_net_dhcp_cfg;
                OS_TaskNetDhcpInitArgs args = {
                    .net_itf_hd = net_itf_hd
                };
                //Start DHCP client to obtain network address.
                IF_STATUS(s = OS_TaskCreate(&args, &task_net_dhcp_cfg, OS_NULL)) {}
            } else {
#endif //(OS_NETWORK_DHCP)
                //Inform network DHCP server about our interface manual configuration.
                IF_STATUS(s = OS_NetworkItfDhcpServerInform(net_itf_hd)) {}
#if (OS_NETWORK_DHCP)
            }
#endif //(OS_NETWORK_DHCP)
        }
    } else { OS_LOG_S(L_WARNING, s); }
    return s;
}

#endif //(OS_NETWORK_ENABLED)
