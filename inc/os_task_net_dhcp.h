/***************************************************************************//**
* @file    os_task_net_dhcp.h
* @brief   Network daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_NET_DHCP_H_
#define _OS_TASK_NET_DHCP_H_

#include "os_config.h"

#if (OS_NETWORK_ENABLED) && (OS_NETWORK_DHCP)
#define OS_DAEMON_NAME_NET_DHCP     "NetDhcpD"

typedef struct {
    OS_NetworkItfHd net_itf_hd;
} OS_TaskNetDhcpInitArgs;
#endif //(OS_NETWORK_ENABLED) && (OS_NETWORK_DHCP)

#endif //_OS_TASK_NET_DHCP_H_