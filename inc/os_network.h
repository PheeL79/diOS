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
/**
* \defgroup OS_Network OS_Network
* @{
*/
//------------------------------------------------------------------------------
#if (OS_NETWORK_IP_V4)
#   define OS_NETWORK_IP4_ADDR             IP4_ADDR
#   define OS_NETWORK_IP4_ADDR_TYPE        IP_ADDR4
#   define OS_NETWORK_IP4_ADDR_INIT        IPADDR4_INIT
#   define OS_NETWORK_IP4_ADDR_INIT_BYTES  IPADDR4_INIT_BYTES
#endif //(OS_NETWORK_IP_V4)

#if (OS_NETWORK_IP_V6)
#   define OS_NETWORK_IP6_ADDR             IP6_ADDR
#   define OS_NETWORK_IP6_ADDR_TYPE        IP_ADDR6
#   define OS_NETWORK_IP6_ADDR_INIT        IPADDR6_INIT
#   define OS_NETWORK_IP6_ADDR_INIT_HOST   IPADDR6_INIT_HOST
#endif //(OS_NETWORK_IP_V6)

//------------------------------------------------------------------------------
typedef struct pbuf             OS_NetworkBuf;
typedef struct netif            OS_NetworkItf;
typedef void*                   OS_NetworkItfHd;
typedef U8                      OS_NetworkItfId;

typedef U8*                     OS_NetworkMacAddr;
typedef struct ip_addr          OS_NetworkIpAddr;
typedef OS_NetworkIpAddr        OS_NetworkNetMask;
typedef OS_NetworkIpAddr        OS_NetworkGateWay;
#if (OS_NETWORK_IP_V4)
typedef ip4_addr_t              OS_NetworkIpAddr4;
typedef OS_NetworkIpAddr4       OS_NetworkNetMask4;
typedef OS_NetworkIpAddr4       OS_NetworkGateWay4;
#endif //(OS_NETWORK_IP_V4)
#if (OS_NETWORK_IP_V6)
typedef ip6_addr_t              OS_NetworkIpAddr6;
#endif //(OS_NETWORK_IP_V6)
typedef enum {
    OS_NET_IP_VERSION_4         = IPADDR_TYPE_V4,
    OS_NET_IP_VERSION_6         = IPADDR_TYPE_V6,
    OS_NET_IP_VERSION_ANY       = IPADDR_TYPE_ANY
} OS_NetworkIpVersion;

typedef enum {
    S_NET_UNDEF = S_MODULE,
    S_NET_ROUTING,
    S_NET_ITF_LL
} OS_NetworkStatus;

enum {
    OS_NET_ITF_FLAG_UP          = NETIF_FLAG_UP,
    OS_NET_ITF_FLAG_BROADCAST   = NETIF_FLAG_BROADCAST,
    OS_NET_ITF_FLAG_LINK_UP     = NETIF_FLAG_LINK_UP,
    OS_NET_ITF_FLAG_ETHARP      = NETIF_FLAG_ETHARP,
    OS_NET_ITF_FLAG_ETHERNET    = NETIF_FLAG_ETHERNET,
    OS_NET_ITF_FLAG_IGMP        = NETIF_FLAG_IGMP,
    OS_NET_ITF_FLAG_MLD6        = NETIF_FLAG_MLD6
};
typedef U8 OS_NetworkItfFlags;

enum {
//ETH Common
    OS_SIG_NET_ETH_LINK_UP = OS_SIG_APP,
    OS_SIG_NET_ETH_LINK_DOWN,
    OS_SIG_NET_ETH_LINK_STATE_CHANGED,
    OS_SIG_NET_ETH_CONN_STATE_CHANGED,
    OS_SIG_NET_ETH_RX,
    OS_SIG_NET_ETH_TX,
    OS_SIG_NET_LAST
};

typedef struct {
    Str                         name[OS_NETWORK_ITF_NAME_LEN];
    OS_DriverConfig*            drv_cfg_p;
    U8                          net_itf_id; //OS_NETWORK_ITF
    Bool                        is_loopback;
} OS_NetworkItfConfig;

typedef struct {
    ConstStrP                   hostname_sp;
    StrP                        name_sp;
    U16                         mtu;
    OS_NetworkItfId             id;
    OS_NetworkItfFlags          flags;
    Bool                        is_loopback;
} OS_NetworkItfDesc;

typedef struct {
    Size                        received_octets;
    Size                        sent_octets;
#if (OS_NETWORK_SNMP)
#if (OS_NETWORK_MIB2_STATS)
    U8                          link_type;
    U32                         link_speed;
    U32                         packets_in;
    U32                         packets_uni_in;
    U32                         packets_octets_in;
    U32                         packets_discard_in;
    U32                         packets_out;
    U32                         packets_uni_out;
    U32                         packets_octets_out;
    U32                         packets_discard_out;
#endif //(OS_NETWORK_MIB2_STATS)
#endif //(OS_NETWORK_SNMP)
} OS_NetworkItfStats;

typedef struct {
    StrP                        host_name_p;
    OS_NetworkMacAddr*          mac_addr_p;
#if (OS_NETWORK_IP_V4)
    OS_NetworkIpAddr4*          ip_addr4_p;
    OS_NetworkNetMask4*         netmask4_p;
    OS_NetworkGateWay4*         gateway4_p;
#endif //(OS_NETWORK_IP_V4)
#if (OS_NETWORK_IP_V6)
    OS_NetworkIpAddr6*          ip_addr6_p;
#endif //(OS_NETWORK_IP_V6)
} OS_NetworkItfInitArgs;

typedef struct {
    OS_QueueHd                  netd_stdin_qhd;
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

/// @brief      Get current state of the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Bool.
Bool            OS_NetworkItfLinkStateGet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Refresh state the network interface.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfLinkStateRefresh(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface link up.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfLinkUpSet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface link down.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfLinkDownSet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface up.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfUpSet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Set the network interface down.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfDownSet(const OS_NetworkItfHd net_itf_hd);

#if (OS_NETWORK_DHCP)
Status          OS_NetworkItfDhcpClientStart(const OS_NetworkItfHd net_itf_hd);
Status          OS_NetworkItfDhcpClientReleaseStop(const OS_NetworkItfHd net_itf_hd);
Status          OS_NetworkItfDhcpClientRenew(const OS_NetworkItfHd net_itf_hd);
Status          OS_NetworkItfDhcpServerInform(const OS_NetworkItfHd net_itf_hd);
#endif //(OS_NETWORK_DHCP)

Status          OS_NetworkItfStatsGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkItfStats* net_itf_stats_p);

Status          OS_NetworkItfDescGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkItfDesc* net_itf_desc_p);

ConstStrP       OS_NetworkItfNameGet(const OS_NetworkItfHd net_itf_hd);

/// @brief      Get the network interface MAC address.
/// @param[in]  net_itf_hd      Network interface.
/// @param[out] mac_addr_p      MAC address.
/// @return     #Status.
Status          OS_NetworkItfMacAddrGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkMacAddr mac_addr_p);

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

Status          OS_NetworkArpStaticEntryAppend(const OS_NetworkIpAddr4 ip_addr4, const OS_NetworkMacAddr mac_addr);
Status          OS_NetworkArpStaticEntryRemove(const OS_NetworkIpAddr4 ip_addr4);

Status          OS_NetworkMacAddressStrToBin(ConstStrP mac_addr_str_p, OS_NetworkMacAddr mac_addr);
#if (OS_NETWORK_IP_V4)
Status          OS_NetworkIpAddress4StrToBin(ConstStrP ip_addr4_str_p, OS_NetworkIpAddr4* ip_addr4_p);
Status          OS_NetworkNetMask4StrToBin(ConstStrP netmask4_str_p, OS_NetworkNetMask4* netmask4_p);
Status          OS_NetworkGateWay4StrToBin(ConstStrP gateway4_str_p, OS_NetworkGateWay4* gateway4_p);
#endif //(OS_NETWORK_IP_V4)
#if (OS_NETWORK_IP_V6)
Status          OS_NetworkIpAddress6StrToBin(ConstStrP ip_addr6_str_p, OS_NetworkIpAddr6* ip_addr6_p);
#endif //(OS_NETWORK_IP_V6)

#if (OS_NETWORK_NETBIOS)
Status          OS_NetworkNetBiosStart(void);
Status          OS_NetworkNetBiosStop(void);
#endif //(OS_NETWORK_NETBIOS)

#if (OS_NETWORK_PERF)
Status          OS_NetworkIperfStart(void** iperf_sess_pp);
Status          OS_NetworkIperfStop(void* iperf_sess_p);
#endif //(OS_NETWORK_PERF)

#if (OS_NETWORK_SNMP)
Status          OS_NetworkSnmpStart(void);
Status          OS_NetworkSnmpStop(void);
#endif //(OS_NETWORK_SNMP)

#if (OS_NETWORK_SNTP)
Status          OS_NetworkSntpStart(void);
Status          OS_NetworkSntpStop(void);
Status          OS_NetworkSntpDateTimeSet(const OS_TimeS utc_timestamp_s);
#endif //(OS_NETWORK_SNTP)

#if (OS_NETWORK_HTTPD)
Status          OS_NetworkHttpDStart(void);
Status          OS_NetworkHttpDStop(void);
#endif //(OS_NETWORK_HTTPD)

/**@}*/ //OS_Network

#endif //(OS_NETWORK_ENABLED)

#ifdef __cplusplus
}
#endif

#endif //_OS_NETWORK_H_