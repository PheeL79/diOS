/***************************************************************************//**
* @file    os_network.c
* @brief   OS Network.
* @author  A. Filyanov
*******************************************************************************/
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
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
} OS_NetworkItfConfigDyn;

//------------------------------------------------------------------------------
static OS_MutexHd os_net_mutex;
static volatile OS_NetworkItfConfigDyn* net_itf_v[OS_NETWORK_ITF_LAST];
static volatile OS_NetworkItfHd def_net_itf_hd;

/******************************************************************************/
static OS_NetworkItfConfigDyn* OS_NetworkItfConfigDynGet(const OS_NetworkItfHd net_itf_hd);
INLINE OS_NetworkItfConfigDyn* OS_NetworkItfConfigDynGet(const OS_NetworkItfHd net_itf_hd)
{
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    return ((OS_NetworkItfConfigDyn*)net_itf_hd);
}

/******************************************************************************/
static ConstStrP OS_NetworkItfNameGet(const OS_NetworkItfHd net_itf_hd);
ConstStrP OS_NetworkItfNameGet(const OS_NetworkItfHd net_itf_hd)
{
    if (OS_NULL == net_itf_hd) { return OS_NULL; }
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    return cfg_dyn_p->name;
}

/*****************************************************************************/
Status OS_NetworkInit(void)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init");
    def_net_itf_hd = OS_NULL;
    os_net_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_net_mutex) { return S_INVALID_REF; }
    OS_MemSet(net_itf_v, 0, sizeof(net_itf_v));
s = S_OK;
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
    if (OS_NULL == cfg_p) { return S_INVALID_REF; }
    if (OS_NULL != net_itf_v[cfg_p->net_itf_id]) { return S_INVALID_VALUE; }
    OS_NetworkItfConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_NetworkItfConfigDyn));
    if (OS_NULL == cfg_dyn_p) { return S_NO_MEMORY; }
    cfg_dyn_p->net_itf_p = OS_Malloc(sizeof(OS_NetworkItf));
    if (OS_NULL == cfg_dyn_p->net_itf_p) {
        OS_Free(cfg_dyn_p);
        return S_NO_MEMORY;
    }
    IF_STATUS(s = OS_DriverCreate(cfg_p->drv_cfg_p, &cfg_dyn_p->dhd)) {
        OS_Free(cfg_dyn_p->net_itf_p);
        OS_Free(cfg_dyn_p);
        return s;
    }
    cfg_dyn_p->net_itf_id = cfg_p->net_itf_id;
    OS_StrNCpy(cfg_dyn_p->name, (const char*)cfg_p->name, sizeof(cfg_dyn_p->name));
    if (OS_NULL != net_itf_hd_p) {
        *net_itf_hd_p = (OS_NetworkItfHd)cfg_dyn_p;
    }
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        net_itf_v[cfg_dyn_p->net_itf_id] = cfg_dyn_p;
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
//error:
    IF_STATUS(s) {
        Status s_drv;
        IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
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
        IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
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
static err_t ethernetif_init(OS_NetworkItf* net_itf_p);
err_t ethernetif_init(OS_NetworkItf* net_itf_p)
{
    return ERR_OK;
}

/*****************************************************************************/
/**
  * @brief  Link callback function, this function is called on change of link status
  *         to update low level driver configuration.
  * @param  netif: The network interface
  * @retval None
  */
static void ethernetif_update_config(OS_NetworkItf* net_itf_p);
void ethernetif_update_config(OS_NetworkItf* net_itf_p)
{
const OS_NetworkItfHd net_itf_hd = OS_NetworkItfHdByIdGet(net_itf_p->num);
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
Status s = S_UNDEF;
    (void)net_itf_p;
    IF_STATUS(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_SETUP, (void*)cfg_dyn_p->net_itf_p)) {
        OS_LOG_S(D_WARNING, s);
    }
}

/*****************************************************************************/
Status OS_NetworkItfInit(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfInitArgs* init_args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_ASSERT_VALUE(OS_NULL != init_args_p);
    OS_LOG(D_DEBUG, "%s: itf init", OS_NetworkItfNameGet(net_itf_hd));
    {
        const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
        const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
        OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
        OS_DriverStats stats;
        IF_STATUS(s = OS_DriverStatsGet(drv_net_itf_hd, &stats)) { return s; }
        if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_INIT))) {
            /* Create tcp_ip stack thread */
            tcpip_init(OS_NULL, OS_NULL);
            /* add the network interface */
#if LWIP_ARP
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
            netifapi_netif_add(net_itf_p, init_args_p->ip4_addr_p, init_args_p->netmask_p, init_args_p->gateway_p,
                               OS_NULL, &ethernetif_init, &tcpip_input);
            if (OS_NULL == OS_NetworkItfDefaultGet()) {
                IF_STATUS(s = OS_NetworkItfDefaultSet(net_itf_hd)) { return s; }
            }
            IF_OK(s = OS_NetworkItfDownSet(net_itf_hd)) {}
            /* Set the link callback function, this function is called on change of link status*/
            netif_set_link_callback(net_itf_p, ethernetif_update_config);
            /* set netif MAC hardware address length */
            net_itf_p->hwaddr_len = ETH_MAC_ADDR_SIZE;
            /* set netif MAC hardware address */
            OS_MemCpy(net_itf_p->hwaddr, init_args_p->mac_addr_p, sizeof(net_itf_p->hwaddr));
            /* set netif maximum transfer unit */
            net_itf_p->mtu = ETH_MTU_SIZE;
#endif  /* LWIP_ARP */
            /* We directly use etharp_output() here to save a function call.
            * You can instead declare your own function an call etharp_output()
            * from it if you have to do some checks before sending (e.g. if link
            * is available...) */
#if LWIP_ARP || LWIP_ETHERNET
#if LWIP_ARP
            net_itf_p->output = etharp_output;
#else
            /* The user should write ist own code in low_level_output_arp_off function */
            net_itf_p->output = low_level_output_arp_off;
#endif /* LWIP_ARP */
#endif  /* LWIP_ARP || LWIP_ETHERNET */
            net_itf_p->linkoutput = low_level_output;
#if LWIP_NETIF_HOSTNAME
            /* Initialize interface hostname */
            net_itf_p->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
            /* Define those to better describe your network interface. */
            net_itf_p->name[0]  = 'e';
            net_itf_p->name[1]  = 'h';
            net_itf_p->num      = cfg_dyn_p->net_itf_id;
            /* Accept broadcast address and ARP traffic */
            /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
#if LWIP_ARP
            net_itf_p->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
#else
            net_itf_p->flags |= NETIF_FLAG_BROADCAST;
#endif /* LWIP_ARP */
            IF_STATUS(s = OS_DriverInit(drv_net_itf_hd, (void*)init_args_p)) { return s; }
        }
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfDeInit(const OS_NetworkItfHd net_itf_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_LOG(D_DEBUG, "%s: itf deinit", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    netifapi_netif_remove(cfg_dyn_p->net_itf_p);
    IF_STATUS(s = OS_DriverIoCtl(drv_net_itf_hd, DRV_REQ_STD_SYNC, args_p)) {
        OS_LOG_S(D_WARNING, s);
    }
    IF_STATUS(s = OS_DriverDeInit(drv_net_itf_hd, args_p)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfOpen(const OS_NetworkItfHd net_itf_hd, const OS_NetworkItfOpenArgs* open_args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_LOG(D_DEBUG, "%s: itf open", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    OS_DriverStats stats;
    IF_STATUS(s = OS_DriverStatsGet(drv_net_itf_hd, &stats)) { return s; }
    if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_OK(s = OS_DriverOpen(drv_net_itf_hd, (void*)open_args_p)) {
        }
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkItfClose(const OS_NetworkItfHd net_itf_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_LOG(D_DEBUG, "%s: itf close", OS_NetworkItfNameGet(net_itf_hd));
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd drv_net_itf_hd = cfg_dyn_p->dhd;
    OS_DriverStats stats;
    IF_STATUS(s = OS_DriverStatsGet(drv_net_itf_hd, &stats)) { return s; }
    if (BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverClose(drv_net_itf_hd, args_p)) { return s; }
        if (1 > OS_DriverOwnersCountGet(drv_net_itf_hd)) {
            if (net_itf_hd == OS_NetworkItfDefaultGet()) {
                IF_STATUS(s = OS_NetworkItfDefaultSet(OS_NULL)) { return s; }
            }
        }
        if (OS_NetworkItfLinkStateGet(net_itf_hd)) {
            s = OS_NetworkItfDownSet(net_itf_hd);
        }
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
Status OS_NetworkItfLinkStateSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
Bool link_status;
Status s = S_UNDEF;
    IF_OK(s = OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_ETH_LINK_STATUS_GET, (void*)&link_status)) {
        if (link_status) {
            if (!OS_NetworkItfLinkStateGet(net_itf_hd)) {
                IF_OK(s = OS_NetworkItfLinkUpSet(net_itf_hd)) {}
            }
        } else {
            if (OS_NetworkItfLinkStateGet(net_itf_hd)) {
                IF_OK(s = OS_NetworkItfLinkDownSet(net_itf_hd)) {}
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
    OS_LOG(D_DEBUG, "%s: link is up", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfLinkDownSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    netif_set_link_down(net_itf_p);
    OS_LOG(D_DEBUG, "%s: link is down", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfUpSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    /* When the netif is fully configured this function must be called.*/
    netifapi_netif_set_up(net_itf_p);
    OS_LOG(D_DEBUG, "%s: itf is up", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfDownSet(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    /* When the netif link is down this function must be called */
    netifapi_netif_set_down(net_itf_p);
    OS_LOG(D_DEBUG, "%s: itf is down", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

#if (LWIP_DHCP)
/*****************************************************************************/
Status OS_NetworkItfDhcpStart(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    netifapi_dhcp_start(net_itf_p);
    OS_LOG(D_DEBUG, "%s: DHCP started", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfDhcpStop(const OS_NetworkItfHd net_itf_hd)
{
const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    netifapi_dhcp_stop(net_itf_p);
    OS_LOG(D_DEBUG, "%s: DHCP stopped", OS_NetworkItfNameGet(net_itf_hd));
    return S_OK;
}
#endif //(LWIP_DHCP)

/*****************************************************************************/
Status OS_NetworkItfAddressGet(const OS_NetworkItfHd net_itf_hd,
                               OS_NetworkIp4Addr* ip4_addr_p,
                               OS_NetworkNetMask* netmask_p,
                               OS_NetworkGateWay* gateway_p)
{
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_ASSERT_VALUE(OS_NULL != ip4_addr_p);
    OS_ASSERT_VALUE(OS_NULL != netmask_p);
    OS_ASSERT_VALUE(OS_NULL != gateway_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    *ip4_addr_p = net_itf_p->ip_addr;
    *netmask_p  = net_itf_p->netmask;
    *gateway_p  = net_itf_p->gw;
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkItfAddressSet(const OS_NetworkItfHd net_itf_hd,
                               const OS_NetworkIp4Addr* ip4_addr_p,
                               const OS_NetworkNetMask* netmask_p,
                               const OS_NetworkGateWay* gateway_p)
{
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    OS_ASSERT_VALUE(OS_NULL != ip4_addr_p);
    OS_ASSERT_VALUE(OS_NULL != netmask_p);
    OS_ASSERT_VALUE(OS_NULL != gateway_p);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    netifapi_netif_set_addr(cfg_dyn_p->net_itf_p,
                           (OS_NetworkIp4Addr*)ip4_addr_p,
                           (OS_NetworkNetMask*)netmask_p,
                           (OS_NetworkGateWay*)gateway_p);
    OS_LOG(D_DEBUG, "%s: address set\r\nip = %d.%d.%d.%d\r\nnetmask = %d.%d.%d.%d\r\ngateway = %d.%d.%d.%d",
                    OS_NetworkItfNameGet(net_itf_hd),
                    ip4_addr_p[0], ip4_addr_p[1], ip4_addr_p[2], ip4_addr_p[3],
                    netmask_p[0], netmask_p[1], netmask_p[2], netmask_p[3],
                    gateway_p[0], gateway_p[1], gateway_p[2], gateway_p[3]);
    return S_OK;
}

/*****************************************************************************/
OS_NetworkItfHd OS_NetworkItfDefaultGet(void)
{
    return def_net_itf_hd;
}

/*****************************************************************************/
Status OS_NetworkItfDefaultSet(const OS_NetworkItfHd net_itf_hd)
{
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_net_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        def_net_itf_hd = net_itf_hd;
        if (net_itf_hd) {
            const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
            OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
            /*  Registers the default network interface. */
            netifapi_netif_set_default(net_itf_p);
        }
        OS_MutexRecursiveUnlock(os_net_mutex);
    }
s = S_OK;
    return s;
}

/*****************************************************************************/
OS_NetworkItfHd OS_NetworkItfHdByIdGet(const OS_NetworkItfId net_itf_id)
{
    return ((OS_NetworkItfHd)net_itf_v[net_itf_id]);
}

/*****************************************************************************/
Status OS_NetworkRead(const OS_NetworkItfHd net_itf_hd, void* data_in_p, Size size)
{
Status s = S_UNDEF;
    OS_LOG(D_DEBUG, "Net read: 0x%X", net_itf_hd);
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    const OS_NetworkItfConfigDyn* cfg_dyn_p = OS_NetworkItfConfigDynGet(net_itf_hd);
    const OS_DriverHd dhd = cfg_dyn_p->dhd;
    OS_NetworkItf* net_itf_p = cfg_dyn_p->net_itf_p;
    OS_NetworkBuf* buf_p;
    IF_STATUS(s = OS_DriverRead(dhd, data_in_p, size, &buf_p)) {
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    if (ERR_OK != net_itf_p->input(buf_p, net_itf_p)) {
        pbuf_free(buf_p);
    }
    return S_OK;
}

/*****************************************************************************/
Status OS_NetworkWrite(const OS_NetworkItfHd net_itf_hd, void* data_out_p, Size size)
{
Status s = S_UNDEF;
//    OS_LOG(D_DEBUG, "Net write: 0x%X", net_itf_hd);
    OS_ASSERT_VALUE(OS_NULL != net_itf_hd);
    const OS_DriverHd dhd = OS_NetworkItfConfigDynGet(net_itf_hd)->dhd;
    IF_STATUS(s = OS_DriverWrite(dhd, data_out_p, size, OS_NULL)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkMacAddressStrToBin(ConstStrP mac_addr_str_p, OS_NetworkMacAddr mac_addr)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(mac_addr_str_p);
    OS_ASSERT_VALUE(mac_addr);
//TODO(A. Filyanov) More strict parse.
    if (ETH_MAC_ADDR_SIZE != OS_SScanF(mac_addr_str_p, "%x:%x:%x:%x:%x:%x%c",
                                       (U8*)&mac_addr[0], (U8*)&mac_addr[1], (U8*)&mac_addr[2],
                                       (U8*)&mac_addr[3], (U8*)&mac_addr[4], (U8*)&mac_addr[5], OS_NULL)) {
        s = S_SIZE_MISMATCH;
    } else {
        s = S_OK;
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkIp4AddressStrToBin(ConstStrP ip4_addr_str_p, OS_NetworkIp4Addr* ip4_addr_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(ip4_addr_str_p);
    OS_ASSERT_VALUE(ip4_addr_p);
//TODO(A. Filyanov) More strict parse.
    if (OS_NETWORK_IP4_ADDR_SIZE != OS_SScanF(ip4_addr_str_p, "%d.%d.%d.%d%c",
                                              (U8*)&ip4_addr_p[0], (U8*)&ip4_addr_p[1],
                                              (U8*)&ip4_addr_p[2], (U8*)&ip4_addr_p[3], OS_NULL)) {
        s = S_SIZE_MISMATCH;
    } else {
        s = S_OK;
    }
    return s;
}

/*****************************************************************************/
Status OS_NetworkNetMaskStrToBin(ConstStrP netmask_str_p, OS_NetworkNetMask* netmask_p)
{
    return OS_NetworkIp4AddressStrToBin(netmask_str_p, netmask_p);
}

/*****************************************************************************/
Status OS_NetworkGateWayStrToBin(ConstStrP gateway_str_p, OS_NetworkNetMask* gateway_p)
{
    return OS_NetworkIp4AddressStrToBin(gateway_str_p, gateway_p);
}

#endif //(OS_NETWORK_ENABLED)