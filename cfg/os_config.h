/**************************************************************************//**
* @file    os_config.h
* @brief   Config header file for OS.
* @author  A. Filyanov
******************************************************************************/
#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

#include "hal_config_prio.h"
//Ensure that is only used by the compiler, and not the assembler.
#ifdef __ICCARM__
#include "hal_config.h"
#include "os_config_prio.h"
#include "os_memory.h"

//------------------------------------------------------------------------------
//Debug
#define OS_DEBUG_ENABLED                            1

//Tests
#define OS_TEST_ENABLED                             0

//Kernel
#define OS_IS_PREEMPTIVE                            1
#define OS_MPU_ENABLED                              0
#define OS_STATS_ENABLED                            1
#define OS_TASK_DEADLOCK_TEST_ENABLED               1

// Timeouts.
#define OS_TIMEOUT_DEFAULT                          1000U
#define OS_TIMEOUT_POWER                            5000U
#define OS_TIMEOUT_DRIVER                           HAL_TIMEOUT_DRIVER
#define OS_TIMEOUT_MUTEX_LOCK                       1000U
#define OS_TIMEOUT_FS                               1000U

// Ticks.
#define OS_TICK_RATE                                1000U
#define OS_PULSE_RATE                               1000U
#define OS_IDLE_TICKLESS_ENABLED                    0
#define OS_IDLE_TICKS_TO_SLEEP                      1000U
#endif // __ICCARM__

// Priorities.
#define OS_PRIORITY_MIN                             0x00
#define OS_PRIORITY_MAX                             U8_MAX

#define OS_PRIORITY_INT_MAX                         HAL_PRIORITY_INT_MAX
#define OS_PRIORITY_INT_MIN                         HAL_PRIORITY_INT_MIN

#define OS_KERNEL_PRIORITY                          (OS_PRIORITY_INT_MIN + 1)

#ifdef __ICCARM__
//Memory
/// @brief   Memory pool type.
enum {
    OS_MEM_RAM_INT_SRAM,
    OS_MEM_RAM_INT_CCM,
    OS_MEM_RAM_EXT_SRAM,
    OS_MEM_LAST,
    OS_MEM_UNDEF
};
#define OS_MEM_HEAP_SYS                             OS_MEM_RAM_INT_SRAM
#define OS_MEM_HEAP_APP                             OS_MEM_RAM_EXT_SRAM
#define OS_STACK_SIZE_MIN                           0x200
#define OS_HEAP_SIZE                                0x15000 //0x1C000

/*__no_init*/ static U8 pool_int_sram[OS_HEAP_SIZE];            //@ RAM_region;
/*__no_init*/ static U8 pool_int_ccm[HAL_MEM_INT_CCM_SIZE]          @ HAL_MEM_INT_CCM_BASE_ADDRESS;
__no_init static U8 pool_ext_sram[HAL_MEM_EXT_SRAM_SIZE]        @ HAL_MEM_EXT_SRAM_BASE_ADDRESS;

/// @brief   Memory config vector.
static const OS_MemoryDesc memory_cfg_v[] = {
    { (void*)&pool_int_sram,    sizeof(pool_int_sram),      HAL_MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_INT_SRAM,    "SRAM Int." },
    { (void*)&pool_int_ccm,     sizeof(pool_int_ccm),       HAL_MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_INT_CCM,     "CCM"       },
    { (void*)&pool_ext_sram,    sizeof(pool_ext_sram),      HAL_MEM_BLOCK_SIZE_MIN,     OS_MEM_RAM_EXT_SRAM,    "SRAM Ext." },
    { 0,                        0,                          0,                          OS_MEM_LAST,            ""          }
};

// Timers
#define OS_TIMERS_ENABLED                           1
#define OS_TIMERS_QUEUE_LEN                         10
#define OS_TIMERS_STACK_SIZE                        (OS_STACK_SIZE_MIN * 2)

// Events
#define OS_TRIGGERS_ENABLED                         0

// Names length
#define OS_DRIVER_NAME_LEN                          9
#define OS_AUDIO_DEVICE_NAME_LEN                    9
#define OS_NETWORK_ITF_NAME_LEN                     9
#define OS_TASK_NAME_LEN                            12
#define OS_TIMER_NAME_LEN                           12

#define OS_LOG_LEVEL_DEFAULT                        "debug1"
#define OS_LOG_STRING_LEN                           128
#define OS_LOG_TIME_ELAPSED                         9999
#define OS_LOG_FILE_PATH                            "0:/log.txt"

//File system
//Look in ffconf.h for details
#define OS_FILE_SYSTEM_ENABLED                      1
#define OS_FILE_SYSTEM_MAKE_EN                      1
#define OS_FILE_SYSTEM_TINY                         0
#define OS_FILE_SYSTEM_READONLY                     0
#define OS_FILE_SYSTEM_MINIMIZE                     0
#define OS_FILE_SYSTEM_STRFUNC_EN                   1
#define OS_FILE_SYSTEM_FIND_EN                      0
#define OS_FILE_SYSTEM_FASTSEEK_EN                  0
#define OS_FILE_SYSTEM_LABEL_EN                     1
#define OS_FILE_SYSTEM_FORWARD_EN                   0
#define OS_FILE_SYSTEM_CODEPAGE                     866
#define OS_FILE_SYSTEM_STRF_ENCODE                  3
#define OS_FILE_SYSTEM_REL_PATH                     2
#define OS_FILE_SYSTEM_MULTI_PART                   0
#define OS_FILE_SYSTEM_TRIM_EN                      0
#define OS_FILE_SYSTEM_NO_INFO                      0
#define OS_FILE_SYSTEM_WORD_ACCESS                  0
#define OS_FILE_SYSTEM_LOCK                         0
#define OS_FILE_SYSTEM_REENTRANT                    1
#define OS_FILE_SYSTEM_SECTOR_SIZE_MIN              0x200
#define OS_FILE_SYSTEM_SECTOR_SIZE_MAX              0x200
//OS_FILE_SYSTEM_VOLUMES_MAX == OS_MEDIA_VOL_LAST !!!
#define OS_FILE_SYSTEM_VOLUMES_MAX                  4
#define OS_FILE_SYSTEM_VOLUME_STR_LEN               4
#define OS_FILE_SYSTEM_VOLUME_NAME_LEN              12
#define OS_FILE_SYSTEM_LONG_NAMES_ENABLED           3
#define OS_FILE_SYSTEM_LONG_NAMES_LEN               255
#define OS_FILE_SYSTEM_LONG_NAMES_UNICODE           0
#define OS_FILE_SYSTEM_DRV_DELIM                    ':'
#define OS_FILE_SYSTEM_DIR_DELIM                    '/'
#define OS_FILE_SYSTEM_SYNC_OBJ                     OS_MutexHd
#define OS_FILE_SYSTEM_YEAR_BASE                    1980U
#define OS_FILE_SYSTEM_NO_RTC                       0
#define OS_FILE_SYSTEM_NO_RTC_DAY                   1
#define OS_FILE_SYSTEM_NO_RTC_MONTH                 1
#define OS_FILE_SYSTEM_NO_RTC_YEAR                  2015U

//Media
enum OS_MEDIA_VOL {
//        OS_MEDIA_VOL_SDRAM,
//#define OS_MEDIA_VOL_SDRAM                          OS_MEDIA_VOL_SDRAM
//#define OS_MEDIA_VOL_SDRAM_MEM                      OS_MEM_RAM_EXT_SRAM
//#define OS_MEDIA_VOL_SDRAM_SIZE                     0x70000
//#define OS_MEDIA_VOL_SDRAM_BLOCK_SIZE               OS_FILE_SYSTEM_SECTOR_SIZE_MIN

        OS_MEDIA_VOL_SDCARD,
#define OS_MEDIA_VOL_SDCARD                         OS_MEDIA_VOL_SDCARD

//        OS_MEDIA_VOL_USBH_FS,
//#define OS_MEDIA_VOL_USBH_FS                        OS_MEDIA_VOL_USBH_FS
//
//        OS_MEDIA_VOL_USBH_HS,
//#define OS_MEDIA_VOL_USBH_HS                        OS_MEDIA_VOL_USBH_HS

        OS_MEDIA_VOL_LAST                           = OS_FILE_SYSTEM_VOLUMES_MAX
};

//Audio
#define OS_AUDIO_ENABLED                            1
#define OS_AUDIO_OUT_SAMPLE_RATE_DEFAULT            44100
#define OS_AUDIO_OUT_SAMPLE_BITS_DEFAULT            16
#define OS_AUDIO_OUT_CHANNELS_DEFAULT               (OS_AUDIO_CHANNELS_STEREO)
#define OS_AUDIO_OUT_VOLUME_DEFAULT                 (OS_AUDIO_VOLUME_MAX / 2)
#define OS_AUDIO_OUT_DMA_MODE_DEFAULT               OS_AUDIO_DMA_MODE_NORMAL

enum OS_AUDIO_DEV {
        OS_AUDIO_DEV_CS4344,
#define OS_AUDIO_DEV_CS4344                         OS_AUDIO_DEV_CS4344

        OS_AUDIO_DEV_LAST
};

//Network
//Look in lwip/opt.h for details
#define OS_NETWORK_ENABLED                          1

//Platform specific locking
#define OS_NETWORK_SYS_LIGHTWEIGHT_PROT             0
#define OS_NETWORK_NO_SYS                           0
#define OS_NETWORK_NO_SYS_NO_TIMERS                 0
#define OS_NETWORK_COMPAT_MUTEX                     0
#define OS_NETWORK_MEMCPY(dst, src, len)            OS_MemCpy(dst, src, len)
#define OS_NETWORK_SMEMCPY(dst, src, len)           OS_MemCpy(dst, src, len)

//Memory options
#define OS_NETWORK_MEM_LIBC_MALLOC                  0
#define OS_NETWORK_MEMP_MEM_MALLOC                  0
#define OS_NETWORK_MEM_ALIGNMENT                    4
#define OS_NETWORK_MEM_SIZE                         (10*1024)
#define OS_NETWORK_MEMP_SEPARATE_POOLS              0
#define OS_NETWORK_MEMP_OVERFLOW_CHECK              0
#define OS_NETWORK_MEMP_SANITY_CHECK                0
#define OS_NETWORK_MEM_USE_POOLS                    0
#define OS_NETWORK_MEM_USE_POOLS_TRY_BIGGER_POOL    0
#define OS_NETWORK_MEMP_USE_CUSTOM_POOLS            0
#define OS_NETWORK_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0

//Internal Memory Pool Sizes
#define OS_NETWORK_MEMP_NUM_PBUF                    16
#define OS_NETWORK_MEMP_NUM_RAW_PCB                 4
#define OS_NETWORK_MEMP_NUM_UDP_PCB                 4
#define OS_NETWORK_MEMP_NUM_TCP_PCB                 5
#define OS_NETWORK_MEMP_NUM_TCP_PCB_LISTEN          8
#define OS_NETWORK_MEMP_NUM_TCP_SEG                 16
#define OS_NETWORK_MEMP_NUM_REASSDATA               5
#define OS_NETWORK_MEMP_NUM_FRAG_PBUF               15
#define OS_NETWORK_MEMP_NUM_ARP_QUEUE               30
#define OS_NETWORK_MEMP_NUM_IGMP_GROUP              8
#define OS_NETWORK_MEMP_NUM_SYS_TIMEOUT             10
#define OS_NETWORK_MEMP_NUM_NETBUF                  2
#define OS_NETWORK_MEMP_NUM_NETCONN                 4
#define OS_NETWORK_MEMP_NUM_TCPIP_MSG_API           8
#define OS_NETWORK_MEMP_NUM_TCPIP_MSG_INPKT         8
#define OS_NETWORK_MEMP_NUM_SNMP_NODE               50
#define OS_NETWORK_MEMP_NUM_SNMP_ROOTNODE           30
#define OS_NETWORK_MEMP_NUM_SNMP_VARBIND            2
#define OS_NETWORK_MEMP_NUM_SNMP_VALUE              3
#define OS_NETWORK_MEMP_NUM_NETDB                   1
#define OS_NETWORK_MEMP_NUM_LOCALHOSTLIST           1
#define OS_NETWORK_MEMP_NUM_PPPOE_INTERFACES        1

//ARP options
#define OS_NETWORK_ARP                              1
#define OS_NETWORK_ARP_TABLE_SIZE                   10
#define OS_NETWORK_ARP_QUEUEING                     1
#define OS_NETWORK_ETHARP_TRUST_IP_MAC              0
#define OS_NETWORK_ETHARP_SUPPORT_VLAN              1
#define OS_NETWORK_ETHERNET                         (OS_NETWORK_ARP)
#define OS_NETWORK_ETH_PAD_SIZE                     0
#define OS_NETWORK_ETHARP_SUPPORT_STATIC_ENTRIES    0

//IP options
#define OS_NETWORK_IP_FORWARD                       0
#define OS_NETWORK_IP_OPTIONS_ALLOWED               1
#define OS_NETWORK_IP_REASSEMBLY                    1
#define OS_NETWORK_IP_FRAG                          1
#define OS_NETWORK_IP_REASS_MAXAGE                  3
#define OS_NETWORK_IP_REASS_MAX_PBUFS               10
#define OS_NETWORK_IP_FRAG_USES_STATIC_BUF          0

#if (OS_NETWORK_IP_FRAG_USES_STATIC_BUF)
#define OS_NETWORK_IP_FRAG_MAX_MTU                  HAL_ETH_MTU_SIZE
#endif

#define OS_NETWORK_IP_DEFAULT_TTL                   255
#define OS_NETWORK_IP_SOF_BROADCAST                 0
#define OS_NETWORK_IP_SOF_BROADCAST_RECV            0
#define OS_NETWORK_IP_FORWARD_ALLOW_TX_ON_RX_NETIF  0
#define OS_NETWORK_RANDOMIZE_INITIAL_LOCAL_PORTS    0

//ICMP options
#define OS_NETWORK_ICMP                             1
#define OS_NETWORK_ICMP_TTL                         (OS_NETWORK_IP_DEFAULT_TTL)
#define OS_NETWORK_BROADCAST_PING                   0
#define OS_NETWORK_MULTICAST_PING                   0

//RAW options
#define OS_NETWORK_RAW                              0
#define OS_NETWORK_RAW_TTL                          (OS_NETWORK_IP_DEFAULT_TTL)

//DHCP options
#define OS_NETWORK_DHCP                             1
#define OS_NETWORK_DHCP_DOES_ARP_CHECK              0

//AUTOIP options
#define OS_NETWORK_AUTOIP                           0
#define OS_NETWORK_DHCP_AUTOIP_COOP                 0
#define OS_NETWORK_DHCP_AUTOIP_COOP_TRIES           9

//SNMP options
#define OS_NETWORK_SNMP                             0
#define OS_NETWORK_SNMP_CONCURRENT_REQUESTS         1
#define OS_NETWORK_SNMP_TRAP_DESTINATIONS           1
#define OS_NETWORK_SNMP_PRIVATE_MIB                 0
#define OS_NETWORK_SNMP_SAFE_REQUESTS               1
#define OS_NETWORK_SNMP_MAX_OCTET_STRING_LEN        127
#define OS_NETWORK_SNMP_MAX_TREE_DEPTH              15
#define OS_NETWORK_SNMP_MAX_VALUE_SIZE              MAX((OS_NETWORK_SNMP_MAX_OCTET_STRING_LEN) + 1, sizeof(S32) * (OS_NETWORK_SNMP_MAX_TREE_DEPTH))

//IGMP options
#define OS_NETWORK_IGMP                             0

//DNS options
#define OS_NETWORK_DNS                              0
#define OS_NETWORK_DNS_TABLE_SIZE                   4
#define OS_NETWORK_DNS_MAX_NAME_LENGTH              256
#define OS_NETWORK_DNS_MAX_SERVERS                  2
#define OS_NETWORK_DNS_DOES_NAME_CHECK              1
#define OS_NETWORK_DNS_MSG_SIZE                     512
#define OS_NETWORK_DNS_LOCAL_HOSTLIST               0
#define OS_NETWORK_DNS_LOCAL_HOSTLIST_IS_DYNAMIC    0

//UDP options
#define OS_NETWORK_UDP                              1
#define OS_NETWORK_UDPLITE                          0
#define OS_NETWORK_UDP_TTL                          (OS_NETWORK_IP_DEFAULT_TTL)
#define OS_NETWORK_NETBUF_RECVINFO                  0

//TCP options
#define OS_NETWORK_TCP                              1
#define OS_NETWORK_TCP_TTL                          (OS_NETWORK_IP_DEFAULT_TTL)
#define OS_NETWORK_TCP_MSS                          536
#define OS_NETWORK_TCP_WND                          (4 * OS_NETWORK_TCP_MSS)
#define OS_NETWORK_TCP_MAXRTX                       12
#define OS_NETWORK_TCP_SYNMAXRTX                    6
#define OS_NETWORK_TCP_QUEUE_OOSEQ                  (OS_NETWORK_TCP)
#define OS_NETWORK_TCP_CALCULATE_EFF_SEND_MSS       1
#define OS_NETWORK_TCP_SND_BUF                      (2 * OS_NETWORK_TCP_MSS)
#define OS_NETWORK_TCP_SND_QUEUELEN                 ((4 * (OS_NETWORK_TCP_SND_BUF) + (OS_NETWORK_TCP_MSS - 1)) / (OS_NETWORK_TCP_MSS))
#define OS_NETWORK_TCP_SNDLOWAT                     MIN(MAX(((OS_NETWORK_TCP_SND_BUF) / 2), (2 * OS_NETWORK_TCP_MSS) + 1), (OS_NETWORK_TCP_SND_BUF) - 1)
#define OS_NETWORK_TCP_SNDQUEUELOWAT                MAX(((OS_NETWORK_TCP_SND_QUEUELEN) / 2), 5)
#define OS_NETWORK_TCP_OOSEQ_MAX_BYTES              0
#define OS_NETWORK_TCP_OOSEQ_MAX_PBUFS              0
#define OS_NETWORK_TCP_LISTEN_BACKLOG               0
#define OS_NETWORK_TCP_DEFAULT_LISTEN_BACKLOG       0xff
#define OS_NETWORK_TCP_OVERSIZE                     (OS_NETWORK_TCP_MSS)
#define OS_NETWORK_TCP_TIMESTAMPS                   0
#define OS_NETWORK_TCP_WND_UPDATE_THRESHOLD         (OS_NETWORK_TCP_WND / 4)

#define OS_NETWORK_EVENT_API                        0
#define OS_NETWORK_CALLBACK_API                     1

//Pbuf options
#define OS_NETWORK_PBUF_LINK_HLEN                   (14 + OS_NETWORK_ETH_PAD_SIZE)
#define OS_NETWORK_PBUF_POOL_SIZE                   16
#define OS_NETWORK_PBUF_POOL_BUFSIZE                MEM_ALIGN_SIZE(OS_NETWORK_TCP_MSS + 40 + OS_NETWORK_PBUF_LINK_HLEN, OS_NETWORK_MEM_ALIGNMENT)

//Network Interfaces options
#define OS_NETWORK_NETIF_HOSTNAME                   1
#define OS_NETWORK_NETIF_API                        0
#define OS_NETWORK_NETIF_STATUS_CALLBACK            0
#define OS_NETWORK_NETIF_LINK_CALLBACK              0
#define OS_NETWORK_NETIF_REMOVE_CALLBACK            0
#define OS_NETWORK_NETIF_HWADDRHINT                 0
#define OS_NETWORK_NETIF_LOOPBACK                   0
#define OS_NETWORK_LOOPBACK_MAX_PBUFS               0
#define OS_NETWORK_NETIF_LOOPBACK_MULTITHREADING    (0 == OS_NETWORK_NO_SYS)
#define OS_NETWORK_NETIF_TX_SINGLE_PBUF             0

//LOOPIF options
#define OS_NETWORK_HAVE_LOOPIF                      0

//SLIPIF options
#define OS_NETWORK_HAVE_SLIPIF                      0

//Thread options
#define OS_NETWORK_SYS_THREAD_MAX                   6
#define OS_NETWORK_TCPIP_THREAD_NAME                "TcpIpD"
#define OS_NETWORK_TCPIP_THREAD_STACKSIZE           (OS_STACK_SIZE_MIN * 2)
#define OS_NETWORK_TCPIP_THREAD_PRIO                (OS_PRIO_TASK_TCPIP)
#define OS_NETWORK_TCPIP_MBOX_SIZE                  5

#define OS_NETWORK_SLIPIF_THREAD_NAME               "SlipIfD"
#define OS_NETWORK_SLIPIF_THREAD_STACKSIZE          (OS_STACK_SIZE_MIN * 2)
#define OS_NETWORK_SLIPIF_THREAD_PRIO               (OS_PRIO_TASK_SLIP)

#define OS_NETWORK_PPP_THREAD_NAME                  "PppInputD"
#define OS_NETWORK_PPP_THREAD_STACKSIZE             (OS_STACK_SIZE_MIN * 2)
#define OS_NETWORK_PPP_THREAD_PRIO                  (OS_PRIO_TASK_PPP_INPUT)

#define OS_NETWORK_DEFAULT_THREAD_NAME              "LwIpD"
#define OS_NETWORK_DEFAULT_THREAD_STACKSIZE         (OS_STACK_SIZE_MIN * 2)
#define OS_NETWORK_DEFAULT_THREAD_PRIO              (OS_PRIO_TASK_LWIP)

#define OS_NETWORK_DEFAULT_RAW_RECVMBOX_SIZE        0
#define OS_NETWORK_DEFAULT_UDP_RECVMBOX_SIZE        2000
#define OS_NETWORK_DEFAULT_TCP_RECVMBOX_SIZE        2000
#define OS_NETWORK_DEFAULT_ACCEPTMBOX_SIZE          2000

//Sequential layer options
#define OS_NETWORK_TCPIP_CORE_LOCKING               0
#define OS_NETWORK_TCPIP_CORE_LOCKING_INPUT         0
#define OS_NETWORK_NETCONN                          1
#define OS_NETWORK_TCPIP_TIMEOUT                    1

//Socket options
#define OS_NETWORK_SOCKET                           0
#define OS_NETWORK_COMPAT_SOCKETS                   0
#define OS_NETWORK_POSIX_SOCKETS_IO_NAMES           1
#define OS_NETWORK_TCP_KEEPALIVE                    0
#define OS_NETWORK_SO_SNDTIMEO                      0
#define OS_NETWORK_SO_RCVTIMEO                      0
#define OS_NETWORK_SO_RCVBUF                        0
#define OS_NETWORK_RECV_BUFSIZE_DEFAULT             INT_MAX
#define OS_NETWORK_SO_REUSE                         0
#define OS_NETWORK_SO_REUSE_RXTOALL                 0

//Statistics options
#define OS_NETWORK_STATS                            0

#if (OS_NETWORK_STATS)

#define OS_NETWORK_STATS_DISPLAY                    0
#define OS_NETWORK_LINK_STATS                       1
#define OS_NETWORK_ETHARP_STATS                     (OS_NETWORK_ARP)
#define OS_NETWORK_IP_STATS                         1
#define OS_NETWORK_IPFRAG_STATS                     (OS_NETWORK_IP_REASSEMBLY || OS_NETWORK_IP_FRAG)
#define OS_NETWORK_ICMP_STATS                       1
#define OS_NETWORK_IGMP_STATS                       (OS_NETWORK_IGMP)
#define OS_NETWORK_UDP_STATS                        (OS_NETWORK_UDP)
#define OS_NETWORK_TCP_STATS                        (OS_NETWORK_TCP)
#define OS_NETWORK_MEM_STATS                        ((0 == OS_NETWORK_MEM_LIBC_MALLOC) && (0 == OS_NETWORK_MEM_USE_POOLS))
#define OS_NETWORK_MEMP_STATS                       (0 == OS_NETWORK_MEMP_MEM_MALLOC)
#define OS_NETWORK_SYS_STATS                        (0 == OS_NETWORK_NO_SYS)

#endif //(OS_NETWORK_STATS)

#define OS_NETWORK_PROVIDE_ERRNO                    1

//PPP options
#define OS_NETWORK_PPP_SUPPORT                      0
#define OS_NETWORK_PPPOE_SUPPORT                    0
#define OS_NETWORK_PPPOS_SUPPORT                    (OS_NETWORK_PPP_SUPPORT)

#if (OS_NETWORK_PPP_SUPPORT)

#define OS_NETWORK_NUM_PPP                          1
#define OS_NETWORK_PAP_SUPPORT                      0
#define OS_NETWORK_CHAP_SUPPORT                     0
#define OS_NETWORK_MSCHAP_SUPPORT                   0
#define OS_NETWORK_CBCP_SUPPORT                     0
#define OS_NETWORK_CCP_SUPPORT                      0
#define OS_NETWORK_VJ_SUPPORT                       0
#define OS_NETWORK_MD5_SUPPORT                      0

//Timeouts
#define OS_NETWORK_FSM_DEFTIMEOUT                   6       /* Timeout time in seconds */
#define OS_NETWORK_FSM_DEFMAXTERMREQS               2       /* Maximum Terminate-Request transmissions */
#define OS_NETWORK_FSM_DEFMAXCONFREQS               10      /* Maximum Configure-Request transmissions */
#define OS_NETWORK_FSM_DEFMAXNAKLOOPS               5       /* Maximum number of nak loops */
#define OS_NETWORK_UPAP_DEFTIMEOUT                  6       /* Timeout (seconds) for retransmitting req */
#define OS_NETWORK_UPAP_DEFREQTIME                  30      /* Time to wait for auth-req from peer */
#define OS_NETWORK_CHAP_DEFTIMEOUT                  6       /* Timeout time in seconds */
#define OS_NETWORK_CHAP_DEFTRANSMITS                10      /* max # times to send challenge */
#define OS_NETWORK_LCP_ECHOINTERVAL                 0
#define OS_NETWORK_LCP_MAXECHOFAILS                 3
#define OS_NETWORK_PPP_MAXIDLEFLAG                  100
/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define OS_NETWORK_PPP_MTU                          HAL_ETH_MTU_SIZE    /* Default MTU (size of Info field) */
#define OS_NETWORK_PPP_MAXMTU                       HAL_ETH_MTU_SIZE    /* Largest MTU we allow */
#define OS_NETWORK_PPP_MINMTU                       64
#define OS_NETWORK_PPP_MRU                          HAL_ETH_MTU_SIZE    /* default MRU = max length of info field */
#define OS_NETWORK_PPP_MAXMRU                       HAL_ETH_MTU_SIZE    /* Largest MRU we allow */
#define OS_NETWORK_PPP_DEFMRU                       296     /* Try for this */
#define OS_NETWORK_PPP_MINMRU                       128     /* No MRUs below this */

#define OS_NETWORK_MAXNAMELEN                       256     /* max length of hostname or name for auth */
#define OS_NETWORK_MAXSECRETLEN                     256     /* max length of password or secret */

#endif //(OS_NETWORK_PPP_SUPPORT)

//Checksum options
#if (0 == HAL_CRC_ENABLED)
#define OS_NETWORK_CHECKSUM_GEN_IP                  1
#define OS_NETWORK_CHECKSUM_GEN_UDP                 1
#define OS_NETWORK_CHECKSUM_GEN_TCP                 1
#define OS_NETWORK_CHECKSUM_GEN_ICMP                1
#define OS_NETWORK_CHECKSUM_CHECK_IP                1
#define OS_NETWORK_CHECKSUM_CHECK_UDP               1
#define OS_NETWORK_CHECKSUM_CHECK_TCP               1
#define OS_NETWORK_CHECKSUM_ON_COPY                 0
#endif //(HAL_CRC_ENABLED)

//Debugging options
#define OS_NETWORK_DEBUG                            0

#define OS_NETWORK_DEBUG_OFF                        0x00U
#define OS_NETWORK_DEBUG_ON                         0x80U
#define OS_NETWORK_DEBUG_MIN_LEVEL                  0x00U
#define OS_NETWORK_DEBUG_TYPES_ON                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_ETHARP                     OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_NETIF                      OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_PBUF                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_API_LIB                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_API_MSG                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_SOCKETS                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_ICMP                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_IGMP                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_INET                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_IP                         OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_IP_REASS                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_RAW                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_MEM                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_MEMP                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_SYS                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TIMERS                     OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_INPUT                  OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_FR                     OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_RTO                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_CWND                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_WND                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_OUTPUT                 OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_RST                    OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCP_QLEN                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_UDP                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_TCPIP                      OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_PPP                        OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_SLIP                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_DHCP                       OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_AUTOIP                     OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_SNMP_MSG                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_SNMP_MIB                   OS_NETWORK_DEBUG_OFF
#define OS_NETWORK_DEBUG_DNS                        OS_NETWORK_DEBUG_OFF

#define OS_NETWORK_DAEMON_QUEUE_LEN                 6
#define OS_NETWORK_HOST_NAME                        HAL_MB_NAME

#define OS_NETWORK_IP_V4                            4
#define OS_NETWORK_IP_V6                            6
#define OS_NETWORK_VERSION_DEFAULT                  (OS_NETWORK_IP_V4)

#define OS_NETWORK_MAC_ADDR_DEFAULT                 "00:80:E1:00:00:00"
//IPv4
#define OS_NETWORK_IP_ADDR4_SIZE                    4
#define OS_NETWORK_IP_ADDR4_DEFAULT                 "10.137.2.64"
#define OS_NETWORK_NETMASK4_DEFAULT                 "255.255.255.0"
#define OS_NETWORK_GATEWAY4_DEFAULT                 "10.137.2.1"
//IPv6
//#define OS_NETWORK_IP_ADDR6_SIZE                    16
//#define OS_NETWORK_IP_ADDR6_DEFAULT                 "fe80::651c:fa43:b012:f989"
//#define OS_NETWORK_NETMASK6_DEFAULT                 "128"
//#define OS_NETWORK_GATEWAY6_DEFAULT                 "fe80::7db8:95ed:da70:641a"

enum OS_NETWORK_ITF {
        OS_NETWORK_ITF_ETH0,
#define OS_NETWORK_ITF_ETH0                         OS_NETWORK_ITF_ETH0

        OS_NETWORK_ITF_LAST
};

//Settings
#define OS_SETTINGS_ENABLED                         1
#define OS_SETTINGS_BROWSE_ENABLED                  0
#define OS_SETTINGS_BUFFER_LEN                      256
#define OS_SETTINGS_VALUE_LEN                       256
#define OS_SETTINGS_VALUE_DEFAULT                   ""
#define OS_SETTINGS_FILE_PATH                       "0:/config.ini"

//Shell
#define OS_SHELL_HEIGHT                             HAL_STDIO_TERM_HEIGHT
#define OS_SHELL_WIDTH                              HAL_STDIO_TERM_WIDTH
#define OS_SHELL_PROMPT_ROOT                        #
#define OS_SHELL_PROMPT_USER                        $
#define OS_SHELL_PROMPT                             OS_SHELL_PROMPT_ROOT
#define OS_SHELL_CL_LEN                             1024
#define OS_SHELL_CL_ARGS_MAX                        7
//Disable to local terminal edit
#define OS_SHELL_EDIT_ENABLED                       0
#define OS_SHELL_HELP_ENABLED                       1

//Locale
#define OS_LOCALE_DATE_DELIM_EN                     "/"
#define OS_LOCALE_DATE_DELIM_RU                     "."
#define OS_LOCALE_TIME_DELIM_EN                     ":"
#define OS_LOCALE_TIME_DELIM_RU                     OS_LOCALE_TIME_DELIM_EN

// cstdlib
#define OS_AtoI                                     HAL_AtoI
#define OS_StrLen                                   HAL_StrLen
#define OS_StrChr                                   HAL_StrChr
#define OS_StrCmp                                   HAL_StrCmp
#define OS_StrCpy                                   HAL_StrCpy
#define OS_StrCat                                   HAL_StrCat
#define OS_StrToL                                   HAL_StrToL
#define OS_StrToK                                   HAL_StrToK
#define OS_StrToUL                                  HAL_StrToUL
#define OS_StrNCpy                                  HAL_StrNCpy
#define OS_SPrintF                                  HAL_SPrintF
#define OS_SNPrintF                                 HAL_SNPrintF
#define OS_SScanF                                   HAL_SScanF

#endif // __ICCARM__
#endif // _OS_CONFIG_H_