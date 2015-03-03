/***************************************************************************//**
* @file    task_log.c
* @brief   Log\trace\dump task definitions.
* @author  A. Filyanov
*******************************************************************************/
#include "lwip/tcpip.h"
#include "os_environment.h"
#include "os_driver.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_timer.h"
#include "os_settings.h"
#include "os_network.h"
#include "os_task_net.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                    "network_d"

//-----------------------------------------------------------------------------
//Task arguments
typedef struct {
    OS_NetworkItfHd net_itf_hd;
    Bool            dhcp_state;
} TaskStorage;

//------------------------------------------------------------------------------
const OS_TaskConfig task_net_cfg = {
    .name           = OS_DAEMON_NAME_NET,
    .func_main      = OS_TaskMain,
    .func_power     = OS_TaskPower,
    .args_p         = OS_NULL,
    .attrs          = BIT(OS_TASK_ATTR_RECREATE),
    .timeout        = 10,
    .prio_init      = OS_TASK_PRIO_NET,
    .prio_power     = OS_TASK_PRIO_PWR_NET,
    .storage_size   = sizeof(TaskStorage),
    .stack_size     = OS_STACK_SIZE_MIN * 2,
    .stdin_len      = 16,
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
Status s = S_UNDEF;
    tstor_p->dhcp_state = OS_FALSE;
    {
        OS_DriverConfig drv_cfg = {
            .name       = "ETH0",
            .itf_p      = drv_eth_v[DRV_ID_ETH0],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        const OS_NetworkItfConfig net_itf_cfg = {
            .name       = "eth0",
            .drv_cfg_p  = &drv_cfg,
            .net_itf_id = OS_NETWORK_ITF_ETH0
        };
        IF_STATUS(s = OS_NetworkItfCreate(&net_itf_cfg, &(tstor_p->net_itf_hd))) { return s; }
    }
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskStorage* tstor_p = (TaskStorage*)args_p->stor_p;
OS_Message* msg_p;
const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
OS_NetworkIp4Addr ip4_addr  = { 0 };
OS_NetworkNetMask netmask   = { 0 };
OS_NetworkGateWay gateway   = { 0 };
UInt timeout = OS_BLOCK;
Status s = S_UNDEF;

	for(;;) {
        IF_STATUS(s = OS_MessageReceive(stdin_qhd, &msg_p, timeout)) {
            if (S_MODULE != s) {
                OS_LOG_S(D_WARNING, S_UNDEF_MSG);
            }
        } else {
            if (OS_SignalIs(msg_p)) {
                switch (OS_SignalIdGet(msg_p)) {
                    case OS_SIG_ETH_RX:
                        {
                        const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(OS_SignalSrcGet(msg_p));
                            OS_NetworkRead(net_itf_hd, OS_NULL, 0);
                        }
                        break;
                    case OS_SIG_ETH_LINK_STATE_CHANGED:
                        {
                        const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(OS_SignalSrcGet(msg_p));
                            IF_STATUS(s = OS_NetworkItfLinkStateSet(net_itf_hd)) {
                                OS_LOG_S(D_WARNING, s);
                            }
                        }
                        break;
                    case OS_SIG_ETH_CONN_STATE_CHANGED:
                        {
                        const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(OS_SignalSrcGet(msg_p));
                        if (OS_NetworkItfLinkStateGet(net_itf_hd)) {
                            IF_OK(s = OS_NetworkItfUpSet(net_itf_hd)) {
                                IF_OK(s = OS_NetworkItfAddressGet(net_itf_hd, &ip4_addr, &netmask, &gateway)) {
                                    if (!ip4_addr.addr) {
                                        IF_OK(s = OS_NetworkItfDhcpStart(net_itf_hd)) {
                                        tstor_p->dhcp_state = OS_TRUE;
                                        timeout = 200;
                                        } else { OS_LOG_S(D_WARNING, s); }
                                    }
                                }
                            }
                        } else {
                            IF_OK(s = OS_NetworkItfDownSet(net_itf_hd)) {}
                        }
                        }
                        break;
                    default:
                        OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                        break;
                }
            } else {
                switch (msg_p->id) {
                    default:
                        OS_LOG_S(D_DEBUG, S_UNDEF_MSG);
                        break;
                }
                OS_MessageDelete(msg_p); // free message allocated memory
            }
        }
        if (tstor_p->dhcp_state) {
            IF_OK(s = OS_NetworkItfAddressGet(tstor_p->net_itf_hd, &ip4_addr, &netmask, &gateway)) {
                if (ip4_addr.addr) {
                    IF_OK(s = OS_NetworkItfDhcpStop(tstor_p->net_itf_hd)) {
                        OS_LOG(D_DEBUG, "DHCP completed");
                        tstor_p->dhcp_state = OS_FALSE;
                        timeout = OS_BLOCK;
                    }
                }
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
#if (OS_SETTINGS_ENABLED)
            ConstStrP config_path_p = OS_EnvVariableGet("config_file");
            Str net_itf_name_str[] = "eth0";
            Str value[OS_SETTINGS_VALUE_LEN];
            Str net_itf_sect_str[0xFF] = "Network\\";
            OS_StrCat(net_itf_sect_str, net_itf_name_str);
#endif //(OS_SETTINGS_ENABLED)
            {
                U8 mac_addr[ETH_MAC_ADDR_SIZE] = { 0 };
#if (OS_SETTINGS_ENABLED)
                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "mac_addr", &value[0])) { goto error; }
                IF_STATUS(s = OS_NetworkMacAddressStrToBin(value, mac_addr)) { goto error; }
#else
                mac_addr[0] = ETH_MAC_ADDR0;
                mac_addr[1] = ETH_MAC_ADDR1;
                mac_addr[2] = ETH_MAC_ADDR2;
                mac_addr[3] = ETH_MAC_ADDR3;
                mac_addr[4] = ETH_MAC_ADDR4;
                mac_addr[5] = ETH_MAC_ADDR5;
#endif //(OS_SETTINGS_ENABLED)
                mac_addr[0] = 0x02;
                mac_addr[1] = 0x00;
                mac_addr[2] = 0x00;
                mac_addr[3] = 0x00;
                mac_addr[4] = 0x00;
                mac_addr[5] = 0x00;

                OS_NetworkIp4Addr ip4_addr  = { 0 };
                OS_NetworkNetMask netmask   = { 0 };
                OS_NetworkGateWay gateway   = { 0 };
                const OS_NetworkItfInitArgs net_itf_init_args = {
                    .mac_addr_p     = (OS_NetworkMacAddr*)&mac_addr,
                    .ip4_addr_p     = &ip4_addr,
                    .netmask_p      = &netmask,
                    .gateway_p      = &gateway
                };
#if (OS_SETTINGS_ENABLED)
//                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "ip4_addr", &value[0])) { goto error; }
//                IF_STATUS(s = OS_NetworkIp4AddressStrToBin(&value[0], net_itf_open_args.ip4_addr_p)) { goto error; }
//                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "netmask", &value[0])) { goto error; }
//                IF_STATUS(s = OS_NetworkNetMaskStrToBin(&value[0], net_itf_open_args.netmask_p)) { goto error; }
//                IF_STATUS(s = OS_SettingsRead(config_path_p, (ConstStrP)net_itf_sect_str, "gateway", &value[0])) { goto error; }
//                IF_STATUS(s = OS_NetworkGateWayStrToBin(&value[0], net_itf_open_args.gateway_p)) { goto error; }
#else
                IF_STATUS(s = OS_NetworkIp4AddressStrToBin(OS_NETWORK_IP4_ADDR_DEFAULT, net_itf_open_args.ip4_addr_p)) { goto error; }
                IF_STATUS(s = OS_NetworkNetMaskStrToBin(OS_NETWORK_NETMASK_DEFAULT, net_itf_open_args.netmask_p)) { goto error; }
                IF_STATUS(s = OS_NetworkGateWayStrToBin(OS_NETWORK_GATEWAY_DEFAULT, net_itf_open_args.gateway_p)) { goto error; }
#endif //(OS_SETTINGS_ENABLED)
                IP4_ADDR(&ip4_addr,  10, 137,   2,  32);
                IP4_ADDR(&netmask,  255, 255, 255,   0);
                IP4_ADDR(&gateway,   10, 137,   2,   1);
                IF_STATUS(s = OS_NetworkItfInit(tstor_p->net_itf_hd, &net_itf_init_args)) { goto error; }
            }
            {
                const OS_NetworkItfOpenArgs net_itf_open_args = {
                    .netd_stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK)
                };
                IF_STATUS(s = OS_NetworkItfOpen(tstor_p->net_itf_hd, &net_itf_open_args)) { goto error; }
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
error:
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}
