/***************************************************************************//**
* @file    os_network.c
* @brief   OS Network.
* @author  A. Filyanov
*******************************************************************************/
#include "lwip/tcpip.h"
#if (OS_NETWORK_DHCP)
#   include "lwip/dhcp.h"
#endif //(OS_NETWORK_DHCP)
#if (OS_NETWORK_NETBIOS)
#   include "lwip/apps/netbiosns.h"
#endif //(OS_NETWORK_NETBIOS)
#if (OS_NETWORK_SNMP)
#   include "lwip/snmp.h"
#   include "lwip/apps/snmp.h"
#endif //(OS_NETWORK_SNMP)
#if (OS_NETWORK_SNTP)
#   include "lwip/apps/sntp.h"
#endif //(OS_NETWORK_SNTP)
#if (OS_NETWORK_PERF)
#   include "lwip/apps/lwiperf.h"
#endif //(OS_NETWORK_PERF)
#if (OS_NETWORK_HTTPD)
#   include "lwip/apps/httpd.h"
#endif //(OS_NETWORK_HTTPD)
#include "netif/etharp.h"
#include "os_network.h"
#include "os_debug.h"
#include "os_mutex.h"

#if (OS_NETWORK_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "network"

//------------------------------------------------------------------------------
typedef struct {
    Str             name[OS_NETWORK_ITF_NAME_LEN];
    OS_DriverHd     dhd;
    OS_NetworkItf*  net_itf_p;
    U8              net_itf_id; //OS_NETWORK_ITF
    Bool            is_loopback;
} OS_NetworkItfConfigDyn;

//------------------------------------------------------------------------------
/// @brief      Log the network interface v4 address.
/// @param[in]  net_itf_hd      Network interface.
/// @return     #Status.
Status          OS_NetworkItfAddress4Log(const OS_NetworkItfHd net_itf_hd);

/// @brief      Translate LwIP stack error codes to OS status.
/// @param[in]  err             Error code.
/// @return     #Status.
static Status   LwipErrTranslate(const err_t err);

//------------------------------------------------------------------------------
static OS_MutexHd os_net_mutex;
static volatile OS_NetworkItfConfigDyn* net_itf_v[OS_NETWORK_ITF_LAST];
static volatile OS_NetworkItfHd net_itf_def_hd;

/******************************************************************************/
static OS_NetworkItfConfigDyn* OS_NetworkItfConfigDynGet(const OS_NetworkItfHd net_itf_hd);
INLINE OS_NetworkItfConfigDyn* OS_NetworkItfConfigDynGet(const OS_NetworkItfHd net_itf_hd)
{
    OS_ASSERT_DEBUG(net_itf_hd);
    return ((OS_NetworkItfConfigDyn*)net_itf_hd);
}

/*****************************************************************************/
Status OS_NetworkInit(void)
{
Status s = S_OK;
    HAL_LOG(L_INFO, "Init");
    net_itf_def_hd = OS_NULL;
    os_net_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_net_mutex) { return S_INVALID_PTR; }
    OS_MemSet(net_itf_v, 0, sizeof(net_itf_v));
    return s;
}

/*****************************************************************************/
Status OS_NetworkDeInit(void)
{
Status s = S_UNDEF;
    //TODO(A. Filyanov) Close and delete existed network interface handles.
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfCreate(const OS_NetworkItfConfig* cfg_p, OS_NetworkItfHd* net_itf_hd_p)
{
Status s = S_UNDEF;
    if (OS_NULL == cfg_p) { return S_INVALID_PTR; }
    if (net_itf_v[cfg_p->net_itf_id]) { return S_INVALID_VALUE; }
    OS_NetworkItfConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_NetworkItfConfigDyn));
    if (OS_NULL == cfg_dyn_p) { return S_OUT_OF_MEMORY; }
    cfg_dyn_p->net_itf_p = OS_Malloc(sizeof(OS_NetworkItf));
    if (OS_NULL == cfg_dyn_p->net_itf_p) {
        OS_Free(cfg_dyn_p);
        return S_OUT_OF_MEMORY;
    }
    if (cfg_p->drv_cfg_p) {
        IF_STATUS(s = OS_DriverCreate(cfg_p->drv_cfg_p, &cfg_dyn_p->dhd)) {
            OS_Free(cfg_dyn_p->net_itf_p);
            OS_Free(cfg_dyn_p);
            return s;
        }
    } else {
        cfg_dyn_p->dhd = OS_NULL;
        s = S_OK;
    }
    cfg_dyn_p->net_itf_id = cfg_p->net_itf_id;
    cfg_dyn_p->is_loopback= cfg_p->is_loopback;
    OS_StrNCpy(cfg_dyn_p->name, (const char*)cfg_p->name, sizeof(cfg_dyn_p->name));
    if (net_itf_hd_p) {
        *net_itf_hd_p = (OS_NetworkItfHd)cfg_dyn_p;
    }
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        net_itf_v[cfg_dyn_p->net_itf_id] = cfg_dyn_p;
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
//error:
    IF_STATUS(s) {
        Status s_drv;
        if (cfg_dyn_p->dhd) {
            IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
        } else { s = S_OK; }
        OS_Free(cfg_dyn_p->net_itf_p);
        OS_Free(cfg_dyn_p);
        net_itf_v[cfg_p->net_itf_id] = OS_NULL;
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDelete(const OS_NetworkItfHd net_itf_hd)
{
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
        Status s_drv;
        if (cfg_dyn_p->dhd) {
            IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
        } else { s = S_OK; }
        net_itf_v[cfg_dyn_p->net_itf_id] = OS_NULL;
        OS_Free(cfg_dyn_p->net_itf_p);
        OS_Free(cfg_dyn_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    return s;
}

/*****************************************************************************/
static err_t low_level_output(OS_NetworkItf* net_itf_p, OS_NetworkBuf* p);
err_t low_level_output(OS_NetworkItf* net_itf_p, OS_NetworkBuf* p)
{
const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(net_itf_p->num);
    IF_OK(OS_NetworkWrite(net_itf_hd, p, p->tot_len)) {
        return ERR_OK;
    } else {
        return ERR_IF;
    }
}

/*****************************************************************************/
static err_t dummy_init(OS_NetworkItf* net_itf_p);
err_t dummy_init(OS_NetworkItf* net_itf_p)
{
    return ERR_OK;
}

/*****************************************************************************/
Status OS_NetworkItfInit(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfInitArgs* init_args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_ASSERT_DEBUG(init_args_p);
    OS_LOG(L_DEBUG_1, "%s: itf init", OS_NetworkItfNameGet(net_itf_hd));
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
        OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
        if (cfg_dyn_p->is_loopback) {
            if (netif_add(net_itf_p, init_args_p->ip_addr4_p, init_args_p->netmask4_p, init_args_p->gateway4_p,
                          OS_NULL, &dummy_init, &tcpip_input)) {
                extern err_t netif_loopif_init(struct netif *netif);
                IF_OK(s = LwipErrTranslate(netif_loopif_init(net_itf_p))) {
                    net_itf_p->mtu = U16_MAX;
#if (OS_NETWORK_IP_V6)
                    IP_ADDR6_HOST(net_itf_p->ip6_addr, 0, 0, 0, 0x00000001UL);
                    net_itf_p->ip6_addr_state[0] = IP6_ADDR_VALID;
#endif //(OS_NETWORK_IP_V6)
                } else {
                    OS_LOG_S(L_WARNING, s);
                }
            } else {
                s = S_ITF_ERROR;
            }
        } else {
            const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
            const OS_DriverState drv_net_itf_state = OS_DriverStateGet(drv_net_itf_hd);
            if (!BIT_TEST(drv_net_itf_state, BIT(OS_DRV_STATE_IS_INIT))) {
                IF_OK(s = OS_NetworkItfDownSet(net_itf_hd)) {}
#if (OS_NETWORK_MIB2_STATS)
                MIB2_INIT_NETIF(net_itf_p, snmp_ifType_other, OS_NETWORK_LINK_SPEED);
#endif //(OS_NETWORK_MIB2_STATS)
                /* add the network interface */
                /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
                struct ip_addr *netmask, struct ip_addr *gw,
                void *state, err_t (* init)(struct netif *netif),
                err_t (* input)(struct pbuf *p, struct netif *netif))

                Adds your network interface to the netif_list. Allocate a struct
                netif and pass a pointer to this structure as the first argument.
                Give pointers to cleared ip_addr structures when using DHCP,
                or fill them with sane numbers otherwise. The state pointer may be NULL.

                The init function pointer must point to a initialization function for
                your ethernet netif interface. The following code illustrates it's use.*/
                if (netif_add(net_itf_p, init_args_p->ip_addr4_p, init_args_p->netmask4_p, init_args_p->gateway4_p,
                              OS_NULL, &dummy_init, &tcpip_input)) {
                    if (OS_NULL == OS_NetworkItfDefaultGet()) {
                        IF_STATUS(s = OS_NetworkItfDefaultSet(net_itf_hd)) {
                            OS_MutexRecursiveUnlock(os_net_mutex);
                            return s;
                        }
                    }
#if (OS_NETWORK_IP_V6)
                    netif_create_ip6_linklocal_address(net_itf_p, OS_TRUE);
                    IF_STATUS(s = LwipErrTranslate(netif_add_ip6_address(net_itf_p, init_args_p->ip_addr6_p, OS_NULL))) {
                        return s;
                    }
                    netif_set_ip6_autoconfig_enabled(net_itf_p, OS_TRUE);
#endif //(OS_NETWORK_IP_V6)
                    /* set netif MAC hardware address length */
                    net_itf_p->hwaddr_len = HAL_ETH_MAC_ADDR_SIZE;
                    /* set netif MAC hardware address */
                    OS_MemCpy(net_itf_p->hwaddr, init_args_p->mac_addr_p, sizeof(net_itf_p->hwaddr));
                    /* set netif maximum transfer unit */
                    net_itf_p->mtu = HAL_ETH_MTU_SIZE;
                    /* We directly use etharp_output() here to save a function call.
                    * You can instead declare your own function an call etharp_output()
                    * from it if you have to do some checks before sending (e.g. if link
                    * is available...) */
#if ((OS_NETWORK_ARP) || (OS_NETWORK_ETHERNET))
#if (OS_NETWORK_ARP)
                    net_itf_p->output = etharp_output;
#else
                    /* The user should write ist own code in low_level_output_arp_off function */
                    net_itf_p->output = low_level_output_arp_off;
#endif //(OS_NETWORK_ARP)
#endif //((OS_NETWORK_ARP) || (OS_NETWORK_ETHERNET))
                    net_itf_p->linkoutput = low_level_output;
                    /* Define those to better describe your network interface. */
                    net_itf_p->name[0]  = 'e';
                    net_itf_p->name[1]  = 't';
                    net_itf_p->num      = cfg_dyn_p->net_itf_id;
                    /* Accept broadcast address and ARP traffic */
                    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
#if (OS_NETWORK_ARP)
                    net_itf_p->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
#else
                    net_itf_p->flags |= NETIF_FLAG_BROADCAST;
#endif //(OS_NETWORK_ARP)
#if (OS_NETWORK_IGMP)
                    net_itf_p->flags |= NETIF_FLAG_IGMP;
#endif //(OS_NETWORK_IGMP)
                    IF_STATUS(s = OS_DriverInit(drv_net_itf_hd, (void*)init_args_p)) {
                        OS_MutexRecursiveUnlock(os_net_mutex);
                        return s;
                    }
                } else {
                    s = S_ITF_ERROR;
                }
            }
        }
//Common interface data init.
#if (OS_NETWORK_NETIF_HOSTNAME)
        /* Initialize interface hostname */
        if (init_args_p->host_name_p) {
            net_itf_p->hostname = init_args_p->host_name_p;
        }
#endif //(OS_NETWORK_NETIF_HOSTNAME)

        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDeInit(const OS_NetworkItfHd net_itf_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_LOG(L_DEBUG_1, "%s: itf deinit", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        OS_Free((void*)cfg_dyn_p->net_itf_p->hostname);
        netif_remove(cfg_dyn_p->net_itf_p);
        if (drv_net_itf_hd) {
            IF_STATUS(s = OS_DriverDeInit(drv_net_itf_hd, args_p)) {
                OS_LOG_S(L_WARNING, s);
            }
        } else { s = S_OK; }
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfOpen(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfOpenArgs* open_args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_LOG(L_DEBUG_1, "%s: itf open", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    if (drv_net_itf_hd) {
        const OS_DriverState drv_net_itf_state = OS_DriverStateGet(drv_net_itf_hd);
        if (!BIT_TEST(drv_net_itf_state, BIT(OS_DRV_STATE_IS_OPEN))) {
            IF_OK(s = OS_DriverOpen(drv_net_itf_hd, (void*)open_args_p)) {
                IF_OK(s = OS_NetworkItfLinkStateRefresh(net_itf_hd)) {
                }
            }
        }
    } else { s = S_OK; }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfClose(const OS_NetworkItfHd net_itf_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_LOG(L_DEBUG_1, "%s: itf close", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    if (drv_net_itf_hd) {
        const OS_DriverState drv_net_itf_state = OS_DriverStateGet(drv_net_itf_hd);
        if (BIT_TEST(drv_net_itf_state, BIT(OS_DRV_STATE_IS_OPEN))) {
            IF_STATUS(s = OS_DriverIoCtl(drv_net_itf_hd, DRV_REQ_STD_SYNC, args_p)) {
                OS_LOG_S(L_WARNING, s);
            }
            IF_STATUS(s = OS_DriverClose(drv_net_itf_hd, args_p)) { return s; }
            if (1 > OS_DriverOwnersCountGet(drv_net_itf_hd)) {
                if (net_itf_hd == OS_NetworkItfDefaultGet()) {
                    IF_STATUS(s = OS_NetworkItfDefaultSet(OS_NULL)) { return s; }
                }
            }
        }
    } else { s = S_OK; }
    if (OS_NetworkItfLinkStateGet(net_itf_hd)) {
        s = OS_NetworkItfDownSet(net_itf_hd);
    }
    return s;
}

/*****************************************************************************/
Bool OS_NetworkItfLinkStateGet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    return netif_is_link_up(net_itf_p);
}

/*****************************************************************************/
Status OS_NetworkItfLinkStateRefresh(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
Bool link_state;
Status s = S_UNDEF;
    IF_OK(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_LINK_INT_CLEAR, OS_NULL)) {
        IF_OK(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_LINK_STATE_GET, (void*)&link_state)) {
            if (link_state) {
                if (!OS_NetworkItfLinkStateGet(net_itf_hd)) {
                    IF_OK(s = OS_NetworkItfLinkUpSet(net_itf_hd)) {
                        IF_OK(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_SETUP, (void*)&link_state)) {
                            IF_OK(s = OS_NetworkItfUpSet(net_itf_hd)) {}
                        }
                    }
                }
            } else {
                if (OS_NetworkItfLinkStateGet(net_itf_hd)) {
                    IF_OK(s = OS_NetworkItfLinkDownSet(net_itf_hd)) {
                        IF_OK(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_SETUP, (void*)&link_state)) {
                            IF_OK(s = OS_NetworkItfDownSet(net_itf_hd)) {}
                        }
                    }
                }
            }
        }
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfLinkUpSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    netif_set_link_up(net_itf_p);
    OS_LOG(L_DEBUG_1, "%s: link is up", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfLinkDownSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    netif_set_link_down(net_itf_p);
    OS_LOG(L_DEBUG_1, "%s: link is down", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfUpSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        /* When the netif is fully configured this function must be called.*/
        netif_set_up(net_itf_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: itf is up", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDownSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        /* When the netif link is down this function must be called */
        netif_set_down(net_itf_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: itf is down", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}

#if (OS_NETWORK_DHCP)
/*****************************************************************************/
Status OS_NetworkItfDhcpClientStart(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        dhcp_start(net_itf_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: DHCP client started", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDhcpClientReleaseStop(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        dhcp_release_and_stop(net_itf_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: DHCP client stopped", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDhcpClientRenew(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        s = LwipErrTranslate(dhcp_renew(net_itf_p));
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: DHCP client renew lease", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDhcpServerInform(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        dhcp_inform(net_itf_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    OS_LOG(L_DEBUG_1, "%s: DHCP server inform", OS_NetworkItfNameGet(net_itf_hd));
    return s;
}
#endif //(OS_NETWORK_DHCP)

/*****************************************************************************/
Status OS_NetworkItfStatsGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkItfStats* net_itf_stats_p)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
OS_DriverStats drv_stats;
Status s = S_UNDEF;
    IF_OK(s = OS_DriverStatsGet(cfg_dyn_p->dhd, &drv_stats)) {
        net_itf_stats_p->received_octets    = drv_stats.received;
        net_itf_stats_p->sent_octets        = drv_stats.sent;
#if (OS_NETWORK_SNMP)
#if (OS_NETWORK_MIB2_STATS)
        net_itf_stats_p->link_type          = net_itf_p->link_type;
        net_itf_stats_p->link_speed         = net_itf_p->link_speed;
        net_itf_stats_p->packets_in         = net_itf_p->mib2_counters.ifinnucastpkts;
        net_itf_stats_p->packets_uni_in     = net_itf_p->mib2_counters.ifinucastpkts;
        net_itf_stats_p->packets_octets_in  = net_itf_p->mib2_counters.ifinoctets;
        net_itf_stats_p->packets_discard_in = net_itf_p->mib2_counters.ifindiscards;
        net_itf_stats_p->packets_out        = net_itf_p->mib2_counters.ifoutnucastpkts;
        net_itf_stats_p->packets_uni_out    = net_itf_p->mib2_counters.ifoutucastpkts;
        net_itf_stats_p->packets_octets_out = net_itf_p->mib2_counters.ifoutoctets;
        net_itf_stats_p->packets_discard_out= net_itf_p->mib2_counters.ifoutdiscards;
#endif //(OS_NETWORK_MIB2_STATS)
#endif //(OS_NETWORK_SNMP)
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDescGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkItfDesc* net_itf_desc_p)
{
Status s = S_OK;
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_ASSERT_DEBUG(net_itf_desc_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    net_itf_desc_p->hostname_sp         = net_itf_p->hostname;
    net_itf_desc_p->name_sp             = net_itf_p->name;
    net_itf_desc_p->mtu                 = net_itf_p->mtu;
    net_itf_desc_p->id                  = cfg_dyn_p->net_itf_id;
    net_itf_desc_p->flags               = net_itf_p->flags;
    net_itf_desc_p->is_loopback         = cfg_dyn_p->is_loopback;
    return s;
}

/******************************************************************************/
ConstStrP OS_NetworkItfNameGet(const OS_NetworkItfHd net_itf_hd)
{
    if (OS_NULL == net_itf_hd) { return OS_NULL; }
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    return cfg_dyn_p->name;
}

/*****************************************************************************/
Status OS_NetworkItfMacAddrGet(const OS_NetworkItfHd net_itf_hd, OS_NetworkMacAddr mac_addr_p)
{
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_ASSERT_DEBUG(mac_addr_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    OS_MemCpy(mac_addr_p, net_itf_p->hwaddr, net_itf_p->hwaddr_len);
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfAddress4Get(const OS_NetworkItfHd net_itf_hd,
                                OS_NetworkIpAddr4*  ip_addr4_p,
                                OS_NetworkNetMask4* netmask4_p,
                                OS_NetworkGateWay4* gateway4_p)
{
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_ASSERT_DEBUG(ip_addr4_p);
    OS_ASSERT_DEBUG(netmask4_p);
    OS_ASSERT_DEBUG(gateway4_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    *ip_addr4_p = net_itf_p->ip_addr;
    *netmask4_p = net_itf_p->netmask;
    *gateway4_p = net_itf_p->gw;
    return S_OK;
}

/******************************************************************************/
Status OS_NetworkItfAddress4Log(const OS_NetworkItfHd net_itf_hd)
{
OS_NetworkIpAddr4  ip_addr4 = { 0 };
OS_NetworkNetMask4 netmask4 = { 0 };
OS_NetworkGateWay4 gateway4 = { 0 };
Status s = S_UNDEF;
    IF_OK(s = OS_NetworkItfAddress4Get(net_itf_hd, &ip_addr4, &netmask4, &gateway4)) {
        OS_LOG(L_DEBUG_1, "%s: address set:\r\nip      = %3d.%3d.%3d.%3d\r\nnetmask = %3d.%3d.%3d.%3d\r\ngateway = %3d.%3d.%3d.%3d",
               OS_NetworkItfNameGet(net_itf_hd),
               ((U8*)&ip_addr4)[0], ((U8*)&ip_addr4)[1], ((U8*)&ip_addr4)[2], ((U8*)&ip_addr4)[3],
               ((U8*)&netmask4)[0], ((U8*)&netmask4)[1], ((U8*)&netmask4)[2], ((U8*)&netmask4)[3],
               ((U8*)&gateway4)[0], ((U8*)&gateway4)[1], ((U8*)&gateway4)[2], ((U8*)&gateway4)[3]);
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfAddress4Set(const OS_NetworkItfHd net_itf_hd,
                                const OS_NetworkIpAddr4*  ip_addr4_p,
                                const OS_NetworkNetMask4* netmask4_p,
                                const OS_NetworkGateWay4* gateway4_p)
{
    OS_ASSERT_DEBUG(net_itf_hd);
    OS_ASSERT_DEBUG(ip_addr4_p);
    OS_ASSERT_DEBUG(netmask4_p);
    OS_ASSERT_DEBUG(gateway4_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        netif_set_addr(cfg_dyn_p->net_itf_p,
                       (OS_NetworkIpAddr4*)ip_addr4_p,
                       (OS_NetworkNetMask4*)netmask4_p,
                       (OS_NetworkGateWay4*)gateway4_p);
        OS_MutexRecursiveUnlock(os_net_mutex);
        IF_OK(s = OS_NetworkItfAddress4Log(net_itf_hd)) {}
    }
    return s;
}

/*****************************************************************************/
OS_NetworkItfHd OS_NetworkItfDefaultGet(void)
{
    return net_itf_def_hd;
}

/*****************************************************************************/
Status OS_NetworkItfDefaultSet(const OS_NetworkItfHd net_itf_hd)
{
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        net_itf_def_hd = net_itf_hd;
        if (net_itf_hd) {
            const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
            OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
            /*  Registers the default network interface. */
            netif_set_default(net_itf_p);
        }
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
    return s;
}

/*****************************************************************************/
OS_NetworkItfHd OS_NetworkItfHdByIdGet(const OS_NetworkItfId net_itf_id)
{
OS_NetworkItfHd net_itf_hd = OS_NULL;
    OS_ASSERT_DEBUG(net_itf_id <= sizeof(net_itf_v));
    if (net_itf_id <= sizeof(net_itf_v)) {
        net_itf_hd = (OS_NetworkItfHd)net_itf_v[net_itf_id];
    }
    return net_itf_hd;
}

/*****************************************************************************/
Status OS_NetworkRead(const OS_NetworkItfHd net_itf_hd, void* data_in_p, Size size)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd dhd = cfg_dyn_p->dhd;
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    OS_NetworkBuf* buf_p;
    if (dhd) {
        IF_STATUS(s = OS_DriverRead(dhd, data_in_p, size, (void*)&buf_p)) {
            OS_LOG_S(L_WARNING, s);
            return s;
        }
    }
    if (ERR_OK != net_itf_p->input(buf_p, net_itf_p)) {
        pbuf_free(buf_p);
    }
    OS_LOG(L_DEBUG_1, "read: 0x%X, size %u", net_itf_hd, buf_p->tot_len);
    return s;
}

/*****************************************************************************/
Status OS_NetworkWrite(const OS_NetworkItfHd net_itf_hd, void* data_out_p, Size size)
{
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(net_itf_hd);
    const OS_DriverHd dhd = OS_NetworkItfConfigDynGet(net_itf_hd)->dhd;
    if (dhd) {
        IF_STATUS(s = OS_DriverWrite(dhd, data_out_p, size, OS_NULL)) {
            OS_LOG_S(L_WARNING, s);
        }
    }
    OS_LOG(L_DEBUG_1, "write: 0x%X, size %u", net_itf_hd, size);
    return s;
}

/*****************************************************************************/
Status OS_NetworkArpStaticEntryAppend(const OS_NetworkIpAddr4 ip_addr4, const OS_NetworkMacAddr mac_addr)
{
Status s = S_UNDEF;
//    etharp_add_static_entry();
    return s;
}

/*****************************************************************************/
Status OS_NetworkArpStaticEntryRemove(const OS_NetworkIpAddr4 ip_addr4)
{
Status s = S_UNDEF;
//    etharp_remove_static_entry();
    return s;
}

/*****************************************************************************/
Status OS_NetworkMacAddressStrToBin(ConstStrP mac_addr_str_p, OS_NetworkMacAddr mac_addr)
{
U8 mac_addr_tmp[HAL_ETH_MAC_ADDR_SIZE + sizeof(UInt)] = { 0 };
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(mac_addr_str_p);
    OS_ASSERT_DEBUG(mac_addr);
//TODO(A. Filyanov) More strict parse.
    if (HAL_ETH_MAC_ADDR_SIZE != OS_SScanF(mac_addr_str_p, "%x:%x:%x:%x:%x:%x",
                                       (UInt*)&mac_addr_tmp[0], (UInt*)&mac_addr_tmp[1], (UInt*)&mac_addr_tmp[2],
                                       (UInt*)&mac_addr_tmp[3], (UInt*)&mac_addr_tmp[4], (UInt*)&mac_addr_tmp[5])) {
        s = S_INVALID_SIZE;
    } else {
        for (Int i = 0; i < HAL_ETH_MAC_ADDR_SIZE; ++i) {
            mac_addr[i] = mac_addr_tmp[i];
        }
        s = S_OK;
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkIpAddress4StrToBin(ConstStrP ip_addr4_str_p, OS_NetworkIpAddr4* ip_addr4_p)
{
U8 ip_addr4_tmp[OS_NETWORK_IP_ADDR4_SIZE + sizeof(UInt)] = { 0 };
Status s = S_UNDEF;
    OS_ASSERT_DEBUG(ip_addr4_str_p);
    OS_ASSERT_DEBUG(ip_addr4_p);
//TODO(A. Filyanov) More strict parse.
    if (OS_NETWORK_IP_ADDR4_SIZE != OS_SScanF(ip_addr4_str_p, "%d.%d.%d.%d",
                                              (UInt*)&ip_addr4_tmp[0], (UInt*)&ip_addr4_tmp[1],
                                              (UInt*)&ip_addr4_tmp[2], (UInt*)&ip_addr4_tmp[3])) {
        s = S_INVALID_SIZE;
    } else {
        *(U32*)ip_addr4_p = *(U32*)&ip_addr4_tmp[0];
        s = S_OK;
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkNetMask4StrToBin(ConstStrP netmask4_str_p, OS_NetworkNetMask4* netmask4_p)
{
    return OS_NetworkIpAddress4StrToBin(netmask4_str_p, netmask4_p);
}

/*****************************************************************************/
Status OS_NetworkGateWay4StrToBin(ConstStrP gateway4_str_p, OS_NetworkGateWay4* gateway4_p)
{
    return OS_NetworkIpAddress4StrToBin(gateway4_str_p, gateway4_p);
}

#if (OS_NETWORK_PERF)
/*****************************************************************************/
Status OS_NetworkIperfStart(void** iperf_sess_pp)
{
Status s = S_OK;
    OS_ASSERT_DEBUG(iperf_sess_pp);
    OS_LOG(L_DEBUG_1, "IPERF start");
    *iperf_sess_pp = lwiperf_start_tcp_server_default(OS_NULL, OS_NULL);
    return s;
}

/*****************************************************************************/
Status OS_NetworkIperfStop(void* iperf_sess_p)
{
Status s = S_OK;
    OS_ASSERT_DEBUG(iperf_sess_p);
    OS_LOG(L_DEBUG_1, "IPERF stop");
    lwiperf_abort(iperf_sess_p);
    return s;
}
#endif //(OS_NETWORK_PERF)

#if (OS_NETWORK_NETBIOS)
/*****************************************************************************/
Status OS_NetworkNetBiosStart(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "NetBIOS init");
    netbiosns_init();
    OS_LOG(L_DEBUG_1, "NetBIOS start");
    return s;
}

/*****************************************************************************/
Status OS_NetworkNetBiosStop(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "NetBIOS stop");
    netbiosns_stop();
    return s;
}
#endif //(OS_NETWORK_NETBIOS)

#if (OS_NETWORK_SNMP)
/*****************************************************************************/
Status OS_NetworkSnmpStart(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "SNMP init");
    snmp_init();
    OS_LOG(L_DEBUG_1, "SNMP start");
    return s;
}

/*****************************************************************************/
Status OS_NetworkSnmpStop(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "SNMP stop");
//    OS_TaskDelete(snmp_thd);
    return s;
}
#endif //(OS_NETWORK_SNMP)

#if (OS_NETWORK_SNTP)
/*****************************************************************************/
Status OS_NetworkSntpStart(void)
{
Status s = S_OK;
    /* SNTP operating modes: default is to poll using unicast.
        The mode has to be set before calling sntp_init(). */
    sntp_setoperatingmode(SNTP_OPMODE);
    OS_LOG(L_DEBUG_1, "SNTP init");
    sntp_init();
    OS_LOG(L_DEBUG_1, "SNTP start");
    return s;
}

/*****************************************************************************/
Status OS_NetworkSntpStop(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "SNTP stop");
    sntp_stop();
    return s;
}

/*****************************************************************************/
Status OS_NetworkSntpDateTimeSet(const OS_TimeS utc_timestamp_s)
{
OS_DateTime date_time = { 0 };
Status s = S_UNDEF;
    IF_OK(s = OS_DateTimeUtcConvert(utc_timestamp_s, &date_time)) {
        IF_OK(s = OS_DateSet(OS_DATE_UNDEF, &date_time)) {
            IF_OK(s = OS_TimeSet(OS_TIME_LOCAL, &date_time)) {
                OS_LOG(L_DEBUG_1, "SNTP client date time set");
            }
        }
    }
    return s;
}
#endif //(OS_NETWORK_SNTP)

#if (OS_NETWORK_HTTPD)

#include "os_file_system.h"
#include "lwip/apps/fs.h"

/*****************************************************************************/
int fs_open_custom(struct fs_file *file, const char *name);
int fs_open_custom(struct fs_file *file, const char *name)
{
OS_FileHd fhd;
StrP file_path_sp = OS_Malloc(512);
ConstStr volume_dir_cs[] = OS_NETWORK_HTTPD_PATH;
Status s = S_UNDEF;
    OS_StrCpy(file_path_sp, (char const*)&volume_dir_cs);
    OS_StrCpy((file_path_sp + sizeof(volume_dir_cs) - 1), name);
    IF_OK(s = OS_FileOpen(&fhd, name, BIT(OS_FS_FILE_OP_MODE_READ))) {
        file->data      = OS_NULL;
        file->len       = (Int)OS_FileSizeGet(fhd);
        file->index     = 0;
        file->pextension= fhd;
        return 1;
    }
    OS_LOG_S(L_WARNING, s);
    return 0;
}

/*****************************************************************************/
void fs_close_custom(struct fs_file *file);
void fs_close_custom(struct fs_file *file)
{
Status s = S_UNDEF;
    IF_STATUS(s = OS_FileClose((OS_FileHd*)&file->pextension)) { OS_LOG_S(L_WARNING, s); }
}

/*****************************************************************************/
int fs_read_custom(struct fs_file *file, char *buffer, int count);
int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
    if (file->index < file->len) {
        IF_OK(OS_FileRead(file->pextension, buffer, count)) {
            file->index += count;
        } else {
            count = 0;
        }
    } else {
        count = FS_READ_EOF;
    }
    return count;
}

/*****************************************************************************/
Status OS_NetworkHttpDStart(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "HTTPD init");
    httpd_init();
    OS_LOG(L_DEBUG_1, "HTTPD start");
    return s;
}

/*****************************************************************************/
Status OS_NetworkHttpDStop(void)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "HTTPD stop");
    return s;
}
#endif //(OS_NETWORK_HTTPD)

/******************************************************************************/
Status LwipErrTranslate(const err_t err)
{
Status s = S_UNDEF;
    switch (err) {
        case ERR_OK:
            s = S_OK;
            break;
        default:
            OS_ASSERT(OS_FALSE);
            break;
    }
    return s;
}

#endif //(OS_NETWORK_ENABLED)