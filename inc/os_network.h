/***************************************************************************//**
* @file    os_network.h
* @brief   OS Network.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_NETWORK_H_
#define _OS_NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwipopts.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "os_signal.h"
#include "os_driver.h"

#if (OS_NETWORK_ENABLED)
//------------------------------------------------------------------------------
typedef struct pbuf         OS_NetworkBuf;
typedef struct netif        OS_NetworkItf;
typedef void*               OS_NetworkItfHd;
typedef U8                  OS_NetworkItfId;
typedef U8*                 OS_NetworkMacAddr;
typedef struct ip_addr      OS_NetworkIpAddr4;
typedef OS_NetworkIpAddr4   OS_NetworkNetMask4;
typedef OS_NetworkIpAddr4   OS_NetworkGateWay4;
//typedef struct ip_addr6     OS_NetworkIpAddr6;
//typedef OS_NetworkIpAddr6   OS_NetworkNetMask6;
//typedef OS_NetworkIpAddr6   OS_NetworkGateWay6;

typedef enum {
    S_NET_UNDEF = S_MODULE,
    S_NET_ROUTING,
    S_NET_ITF_LL
} OS_NetworkStatus;

enum {
//ETH Common
    OS_SIG_ETH_LINK_UP = OS_SIG_APP,
    OS_SIG_ETH_LINK_DOWN,
    OS_SIG_ETH_LINK_STATE_CHANGED,
    OS_SIG_ETH_CONN_STATE_CHANGED,
    OS_SIG_ETH_RX,
    OS_SIG_ETH_TX,

    OS_SIG_ETH_LAST
};

typedef struct {
    Str                     name[OS_NETWORK_ITF_NAME_LEN];
    OS_DriverConfig*        drv_cfg_p;
    U8                      net_itf_id; //OS_NETWORK_ITF
} OS_NetworkItfConfig;

typedef struct {
    OS_NetworkMacAddr*      mac_addr_p;
    OS_NetworkIpAddr4*      ip_addr4_p;
    OS_NetworkNetMask4*     netmask4_p;
    OS_NetworkGateWay4*     gateway4_p;
} OS_NetworkItfInitArgs;

typedef struct {
    OS_QueueHd              netd_stdin_qhd;
} OS_NetworkItfOpenArgs;

//------------------------------------------------------------------------------
/// @brief      Init network.
/// @return     #Status.
Status          OS_NetworkInit(void);

/// @brief      DeInit network.
/// @return     #Status.
Status          OS_NetworkDeInit(void);

/// @brief      Create a network interface.
/// @param[in]  net_itf_hd_p    Network interface.
/// @return     #Status.
Status          OS_NetworkItfCreate(const OS_NetworkItfConfig* cfg_p, OS_NetworkItfHd* net_itf_hd_p);

/// @brief      Delete the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfDelete(const OS_NetworkItfHd net_itf_hd);

/// @brief      Init the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfInit(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfInitArgs* init_args_p);

/// @brief      DeInit the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfDeInit(const OS_NetworkItfHd net_itf_hd, void* args_p);

/// @brief      Open the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfOpen(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfOpenArgs* open_args_p);

/// @brief      Close the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfClose(const OS_NetworkItfHd net_itf_hd, void* args_p);

Bool            OS_NetworkItfLinkStateGet(const OS_NetworkItfHd net_itf_hd);

Status          OS_NetworkItfLinkStateSet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface link up.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfLinkUpSet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface link down.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfLinkDownSet(const OS_NetworkItfHd net_itf_hd);

Status          OS_NetworkItfUpSet(const OS_NetworkItfHd net_itf_hd);
Status          OS_NetworkItfDownSet(const OS_NetworkItfHd net_itf_hd);

#if (LWIP_DHCP)
Status          OS_NetworkItfDhcpClientStart(const OS_NetworkItfHd net_itf_hd);
Status          OS_NetworkItfDhcpClientStop(const OS_NetworkItfHd net_itf_hd);
#endif //(LWIP_DHCP)

ConstStrP       OS_NetworkItfNameGet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Get the network interface v4 address.
/// @param[in]  net_itf_hd      Network interface.
/// @param[out] ip_addr4_p      IP address v4.
/// @param[out] netmask4_p      Address mask.
/// @param[out] gateway4_p      Gateway.
/// @return     #Status.
Status          OS_NetworkItfAddress4Get(const OS_NetworkItfHd net_itf_hd,
                                         OS_NetworkIpAddr4*  ip_addr4_p,
                                         OS_NetworkNetMask4* netmask4_p,
                                         OS_NetworkGateWay4* gateway4_p);

/// @brief      Set the network interface v4 address.
/// @param[in]  net_itf_hd      Network interface.
/// @param[in]  ip_addr4_p      IP address v4.
/// @param[in]  netmask4_p      Address mask.
/// @param[in]  gateway4_p      Gateway.
/// @return     #Status.
Status          OS_NetworkItfAddress4Set(const OS_NetworkItfHd net_itf_hd,
                                         const OS_NetworkIpAddr4*  ip_addr4_p,
                                         const OS_NetworkNetMask4* netmask4_p,
                                         const OS_NetworkGateWay4* gateway4_p);

OS_NetworkItfHd OS_NetworkItfDefaultGet(void);

Status          OS_NetworkItfDefaultSet(const OS_NetworkItfHd net_itf_hd);

OS_NetworkItfHd OS_NetworkItfHdByIdGet(const OS_NetworkItfId net_itf_id);

/// @brief      Creates a new connection.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkCreate(void);

/// @brief      Deletes an existing connection.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkDelete(void);

/// @brief      Binds a connection to a local IP address and port.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkBind(void);

/// @brief      Connects to a remote IP address and port.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkConnect(void);

/// @brief      Closes a TCP connection without deleting it.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkClose(void);

/// @brief      Read data from a netconn.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkRead(const OS_NetworkItfHd net_itf_hd, void* data_in_p, Size size);

/// @brief      Sends data on a connected TCP netconn.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkWrite(const OS_NetworkItfHd net_itf_hd, void* data_out_p, Size size);

/// @brief      Sends data to the currently connected remote IP/port (not applicable for TCP connections).
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkSend(void);

/// @brief      Sets a TCP connection into a listening mode.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkListen(void);

/// @brief      Accepts an incoming connection on a listening TCP connection.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_NetworkAccept(void);

Status          OS_NetworkMacAddressStrToBin(ConstStrP mac_addr_str_p, OS_NetworkMacAddr mac_addr);
Status          OS_NetworkIpAddress4StrToBin(ConstStrP ip_addr4_str_p, OS_NetworkIpAddr4* ip_addr4_p);
Status          OS_NetworkNetMask4StrToBin(ConstStrP netmask4_str_p, OS_NetworkNetMask4* netmask4_p);
Status          OS_NetworkGateWay4StrToBin(ConstStrP gateway4_str_p, OS_NetworkGateWay4* gateway4_p);

#endif //(OS_NETWORK_ENABLED)

#ifdef __cplusplus
}
#endif

#endif //_OS_NETWORK_H_