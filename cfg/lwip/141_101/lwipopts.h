/**
 * @file
 *
 * lwIP Options Configuration
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "os_config.h"

//Platform specific locking
#define SYS_LIGHTWEIGHT_PROT            OS_NETWORK_SYS_LIGHTWEIGHT_PROT
#define NO_SYS                          OS_NETWORK_NO_SYS
#define NO_SYS_NO_TIMERS                OS_NETWORK_NO_SYS_NO_TIMERS
#define LWIP_COMPAT_MUTEX               OS_NETWORK_COMPAT_MUTEX
#define MEMCPY(dst,src,len)             OS_NETWORK_MEMCPY(dst,src,len)
#define SMEMCPY(dst,src,len)            OS_NETWORK_SMEMCPY(dst,src,len)
//Memory options
#define MEM_LIBC_MALLOC                 OS_NETWORK_MEM_LIBC_MALLOC
#define MEMP_MEM_MALLOC                 OS_NETWORK_MEMP_MEM_MALLOC
#define MEM_ALIGNMENT                   OS_NETWORK_MEM_ALIGNMENT
#define MEM_SIZE                        OS_NETWORK_MEM_SIZE
#define MEMP_SEPARATE_POOLS             OS_NETWORK_MEMP_SEPARATE_POOLS
#define MEMP_OVERFLOW_CHECK             OS_NETWORK_MEMP_OVERFLOW_CHECK
#define MEMP_SANITY_CHECK               OS_NETWORK_MEMP_SANITY_CHECK
#define MEM_USE_POOLS                   OS_NETWORK_MEM_USE_POOLS
#define MEM_USE_POOLS_TRY_BIGGER_POOL   OS_NETWORK_MEM_USE_POOLS_TRY_BIGGER_POOL
#define MEMP_USE_CUSTOM_POOLS           OS_NETWORK_MEMP_USE_CUSTOM_POOLS
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT OS_NETWORK_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
//Internal Memory Pool Sizes
#define MEMP_NUM_PBUF                   OS_NETWORK_MEMP_NUM_PBUF
#define MEMP_NUM_RAW_PCB                OS_NETWORK_MEMP_NUM_RAW_PCB
#define MEMP_NUM_UDP_PCB                OS_NETWORK_MEMP_NUM_UDP_PCB
#define MEMP_NUM_TCP_PCB                OS_NETWORK_MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB_LISTEN         OS_NETWORK_MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_SEG                OS_NETWORK_MEMP_NUM_TCP_SEG
#define MEMP_NUM_REASSDATA              OS_NETWORK_MEMP_NUM_REASSDATA
#define MEMP_NUM_FRAG_PBUF              OS_NETWORK_MEMP_NUM_FRAG_PBUF
#define MEMP_NUM_ARP_QUEUE              OS_NETWORK_MEMP_NUM_ARP_QUEUE
#define MEMP_NUM_IGMP_GROUP             OS_NETWORK_MEMP_NUM_IGMP_GROUP
#define MEMP_NUM_SYS_TIMEOUT            OS_NETWORK_MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_NETBUF                 OS_NETWORK_MEMP_NUM_NETBUF
#define MEMP_NUM_NETCONN                OS_NETWORK_MEMP_NUM_NETCONN
#define MEMP_NUM_TCPIP_MSG_API          OS_NETWORK_MEMP_NUM_TCPIP_MSG_API
#define MEMP_NUM_TCPIP_MSG_INPKT        OS_NETWORK_MEMP_NUM_TCPIP_MSG_INPKT
#define MEMP_NUM_SNMP_NODE              OS_NETWORK_MEMP_NUM_SNMP_NODE
#define MEMP_NUM_SNMP_ROOTNODE          OS_NETWORK_MEMP_NUM_SNMP_ROOTNODE
#define MEMP_NUM_SNMP_VARBIND           OS_NETWORK_MEMP_NUM_SNMP_VARBIND
#define MEMP_NUM_SNMP_VALUE             OS_NETWORK_MEMP_NUM_SNMP_VALUE
#define MEMP_NUM_NETDB                  OS_NETWORK_MEMP_NUM_NETDB
#define MEMP_NUM_LOCALHOSTLIST          OS_NETWORK_MEMP_NUM_LOCALHOSTLIST
#define MEMP_NUM_PPPOE_INTERFACES       OS_NETWORK_MEMP_NUM_PPPOE_INTERFACES
#define PBUF_POOL_SIZE                  OS_NETWORK_PBUF_POOL_SIZE
//ARP options
#define LWIP_ARP                        OS_NETWORK_ARP
#define ARP_TABLE_SIZE                  OS_NETWORK_ARP_TABLE_SIZE
#define ARP_QUEUEING                    OS_NETWORK_ARP_QUEUEING
#define ETHARP_TRUST_IP_MAC             OS_NETWORK_ETHARP_TRUST_IP_MAC
#define ETHARP_SUPPORT_VLAN             OS_NETWORK_ETHARP_SUPPORT_VLAN
#define LWIP_ETHERNET                   OS_NETWORK_ETHERNET
#define ETH_PAD_SIZE                    OS_NETWORK_ETH_PAD_SIZE
#define ETHARP_SUPPORT_STATIC_ENTRIES   OS_NETWORK_ETHARP_SUPPORT_STATIC_ENTRIES
//IP options
#define IP_FORWARD                      OS_NETWORK_IP_FORWARD
#define IP_OPTIONS_ALLOWED              OS_NETWORK_IP_OPTIONS_ALLOWED
#define IP_REASSEMBLY                   OS_NETWORK_IP_REASSEMBLY
#define IP_FRAG                         OS_NETWORK_IP_FRAG
#define IP_REASS_MAXAGE                 OS_NETWORK_IP_REASS_MAXAGE
#define IP_REASS_MAX_PBUFS              OS_NETWORK_IP_REASS_MAX_PBUFS
#define IP_FRAG_USES_STATIC_BUF         OS_NETWORK_IP_FRAG_USES_STATIC_BUF

//#if IP_FRAG_USES_STATIC_BUF && !defined(IP_FRAG_MAX_MTU)
#define IP_FRAG_MAX_MTU                 OS_NETWORK_IP_FRAG_MAX_MTU
//#endif

#define IP_DEFAULT_TTL                  OS_NETWORK_IP_DEFAULT_TTL
#define IP_SOF_BROADCAST                OS_NETWORK_IP_SOF_BROADCAST
#define IP_SOF_BROADCAST_RECV           OS_NETWORK_IP_SOF_BROADCAST_RECV
#define IP_FORWARD_ALLOW_TX_ON_RX_NETIF OS_NETWORK_IP_FORWARD_ALLOW_TX_ON_RX_NETIF
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS OS_NETWORK_RANDOMIZE_INITIAL_LOCAL_PORTS
//ICMP options
#define LWIP_ICMP                       OS_NETWORK_ICMP
#define ICMP_TTL                        OS_NETWORK_ICMP_TTL
#define LWIP_BROADCAST_PING             OS_NETWORK_BROADCAST_PING
#define LWIP_MULTICAST_PING             OS_NETWORK_MULTICAST_PING
//RAW options
#define LWIP_RAW                        OS_NETWORK_RAW
#define RAW_TTL                         OS_NETWORK_RAW_TTL
//DHCP options
#define LWIP_DHCP                       OS_NETWORK_DHCP
#define DHCP_DOES_ARP_CHECK             OS_NETWORK_DHCP_DOES_ARP_CHECK
//AUTOIP options
#define LWIP_AUTOIP                     OS_NETWORK_AUTOIP
#define LWIP_DHCP_AUTOIP_COOP           OS_NETWORK_DHCP_AUTOIP_COOP
#define LWIP_DHCP_AUTOIP_COOP_TRIES     OS_NETWORK_DHCP_AUTOIP_COOP_TRIES
//SNMP options
#define LWIP_SNMP                       OS_NETWORK_SNMP
#define SNMP_CONCURRENT_REQUESTS        OS_NETWORK_SNMP_CONCURRENT_REQUESTS
#define SNMP_TRAP_DESTINATIONS          OS_NETWORK_SNMP_TRAP_DESTINATIONS
#define SNMP_PRIVATE_MIB                OS_NETWORK_SNMP_PRIVATE_MIB
#define SNMP_SAFE_REQUESTS              OS_NETWORK_SNMP_SAFE_REQUESTS
#define SNMP_MAX_OCTET_STRING_LEN       OS_NETWORK_SNMP_MAX_OCTET_STRING_LEN
#define SNMP_MAX_TREE_DEPTH             OS_NETWORK_SNMP_MAX_TREE_DEPTH
#define SNMP_MAX_VALUE_SIZE             OS_NETWORK_SNMP_MAX_VALUE_SIZE
//IGMP options
#define LWIP_IGMP                       OS_NETWORK_IGMP
//DNS options
#define LWIP_DNS                        OS_NETWORK_DNS
#define DNS_TABLE_SIZE                  OS_NETWORK_DNS_TABLE_SIZE
#define DNS_MAX_NAME_LENGTH             OS_NETWORK_DNS_MAX_NAME_LENGTH
#define DNS_MAX_SERVERS                 OS_NETWORK_DNS_MAX_SERVERS
#define DNS_DOES_NAME_CHECK             OS_NETWORK_DNS_DOES_NAME_CHECK
#define DNS_MSG_SIZE                    OS_NETWORK_DNS_MSG_SIZE
#define DNS_LOCAL_HOSTLIST              OS_NETWORK_DNS_LOCAL_HOSTLIST
#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC   OS_NETWORK_DNS_LOCAL_HOSTLIST_IS_DYNAMIC
//UDP options
#define LWIP_UDP                        OS_NETWORK_UDP
#define LWIP_UDPLITE                    OS_NETWORK_UDPLITE
#define UDP_TTL                         OS_NETWORK_UDP_TTL
#define LWIP_NETBUF_RECVINFO            OS_NETWORK_NETBUF_RECVINFO
//TCP options
#define LWIP_TCP                        OS_NETWORK_TCP
#define TCP_TTL                         OS_NETWORK_TCP_TTL
#define TCP_WND                         OS_NETWORK_TCP_WND
#define TCP_MAXRTX                      OS_NETWORK_TCP_MAXRTX
#define TCP_SYNMAXRTX                   OS_NETWORK_TCP_SYNMAXRTX
#define TCP_QUEUE_OOSEQ                 OS_NETWORK_TCP_QUEUE_OOSEQ
#define TCP_MSS                         OS_NETWORK_TCP_MSS
#define TCP_CALCULATE_EFF_SEND_MSS      OS_NETWORK_TCP_CALCULATE_EFF_SEND_MSS
#define TCP_SND_BUF                     OS_NETWORK_TCP_SND_BUF
#define TCP_SND_QUEUELEN                OS_NETWORK_TCP_SND_QUEUELEN
#define TCP_SNDLOWAT                    OS_NETWORK_TCP_SNDLOWAT
#define TCP_SNDQUEUELOWAT               OS_NETWORK_TCP_SNDQUEUELOWAT
#define TCP_OOSEQ_MAX_BYTES             OS_NETWORK_TCP_OOSEQ_MAX_BYTES
#define TCP_OOSEQ_MAX_PBUFS             OS_NETWORK_TCP_OOSEQ_MAX_PBUFS
#define TCP_LISTEN_BACKLOG              OS_NETWORK_TCP_LISTEN_BACKLOG
#define TCP_DEFAULT_LISTEN_BACKLOG      OS_NETWORK_TCP_DEFAULT_LISTEN_BACKLOG
#define TCP_OVERSIZE                    OS_NETWORK_TCP_OVERSIZE
#define LWIP_TCP_TIMESTAMPS             OS_NETWORK_TCP_TIMESTAMPS
#define TCP_WND_UPDATE_THRESHOLD        OS_NETWORK_TCP_WND_UPDATE_THRESHOLD

//#if !defined(OS_NETWORK_EVENT_API) && !defined(LWIP_CALLBACK_API)
#define LWIP_EVENT_API                  OS_NETWORK_EVENT_API
#define LWIP_CALLBACK_API               OS_NETWORK_CALLBACK_API
//#endif
//Pbuf options
#define PBUF_LINK_HLEN                  OS_NETWORK_PBUF_LINK_HLEN
#define PBUF_POOL_BUFSIZE               OS_NETWORK_PBUF_POOL_BUFSIZE
//Network Interfaces options
#define LWIP_NETIF_HOSTNAME             OS_NETWORK_NETIF_HOSTNAME
#define LWIP_NETIF_API                  OS_NETWORK_NETIF_API
#define LWIP_NETIF_STATUS_CALLBACK      OS_NETWORK_NETIF_STATUS_CALLBACK
#define LWIP_NETIF_LINK_CALLBACK        OS_NETWORK_NETIF_LINK_CALLBACK
#define LWIP_NETIF_REMOVE_CALLBACK      OS_NETWORK_NETIF_REMOVE_CALLBACK
#define LWIP_NETIF_HWADDRHINT           OS_NETWORK_NETIF_HWADDRHINT
#define LWIP_NETIF_LOOPBACK             OS_NETWORK_NETIF_LOOPBACK
#define LWIP_LOOPBACK_MAX_PBUFS         OS_NETWORK_LOOPBACK_MAX_PBUFS
#define LWIP_NETIF_LOOPBACK_MULTITHREADING OS_NETWORK_NETIF_LOOPBACK_MULTITHREADING
#define LWIP_NETIF_TX_SINGLE_PBUF       OS_NETWORK_NETIF_TX_SINGLE_PBUF
//LOOPIF options
#define LWIP_HAVE_LOOPIF                OS_NETWORK_HAVE_LOOPIF
//SLIPIF options
#define LWIP_HAVE_SLIPIF                OS_NETWORK_HAVE_SLIPIF
//Thread options
#define TCPIP_THREAD_NAME               OS_NETWORK_TCPIP_THREAD_NAME
#define TCPIP_THREAD_STACKSIZE          OS_NETWORK_TCPIP_THREAD_STACKSIZE
#define TCPIP_THREAD_PRIO               OS_NETWORK_TCPIP_THREAD_PRIO
#define TCPIP_MBOX_SIZE                 OS_NETWORK_TCPIP_MBOX_SIZE
#define SLIPIF_THREAD_NAME              OS_NETWORK_SLIPIF_THREAD_NAME
#define SLIPIF_THREAD_STACKSIZE         OS_NETWORK_SLIPIF_THREAD_STACKSIZE
#define SLIPIF_THREAD_PRIO              OS_NETWORK_SLIPIF_THREAD_PRIO
#define PPP_THREAD_NAME                 OS_NETWORK_PPP_THREAD_NAME
#define PPP_THREAD_STACKSIZE            OS_NETWORK_PPP_THREAD_STACKSIZE
#define PPP_THREAD_PRIO                 OS_NETWORK_PPP_THREAD_PRIO
#define DEFAULT_THREAD_NAME             OS_NETWORK_DEFAULT_THREAD_NAME
#define DEFAULT_THREAD_STACKSIZE        OS_NETWORK_DEFAULT_THREAD_STACKSIZE
#define DEFAULT_THREAD_PRIO             OS_NETWORK_DEFAULT_THREAD_PRIO
#define DEFAULT_RAW_RECVMBOX_SIZE       OS_NETWORK_DEFAULT_RAW_RECVMBOX_SIZE
#define DEFAULT_UDP_RECVMBOX_SIZE       OS_NETWORK_DEFAULT_UDP_RECVMBOX_SIZE
#define DEFAULT_TCP_RECVMBOX_SIZE       OS_NETWORK_DEFAULT_TCP_RECVMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         OS_NETWORK_DEFAULT_ACCEPTMBOX_SIZE
//Sequential layer options
#define LWIP_TCPIP_CORE_LOCKING         OS_NETWORK_TCPIP_CORE_LOCKING
#define LWIP_TCPIP_CORE_LOCKING_INPUT   OS_NETWORK_TCPIP_CORE_LOCKING_INPUT
#define LWIP_NETCONN                    OS_NETWORK_NETCONN
#define LWIP_TCPIP_TIMEOUT              OS_NETWORK_TCPIP_TIMEOUT
//Socket options
#define LWIP_SOCKET                     OS_NETWORK_SOCKET
#define LWIP_COMPAT_SOCKETS             OS_NETWORK_COMPAT_SOCKETS
#define LWIP_POSIX_SOCKETS_IO_NAMES     OS_NETWORK_POSIX_SOCKETS_IO_NAMES
#define LWIP_TCP_KEEPALIVE              OS_NETWORK_TCP_KEEPALIVE
#define LWIP_SO_SNDTIMEO                OS_NETWORK_SO_SNDTIMEO
#define LWIP_SO_RCVTIMEO                OS_NETWORK_SO_RCVTIMEO
#define LWIP_SO_RCVBUF                  OS_NETWORK_SO_RCVBUF
#define RECV_BUFSIZE_DEFAULT            OS_NETWORK_RECV_BUFSIZE_DEFAULT
#define SO_REUSE                        OS_NETWORK_SO_REUSE
#define SO_REUSE_RXTOALL                OS_NETWORK_SO_REUSE_RXTOALL
//Statistics options
#define LWIP_STATS                      OS_NETWORK_STATS

#if (LWIP_STATS)

#define LWIP_STATS_DISPLAY              OS_NETWORK_STATS_DISPLAY
#define LINK_STATS                      OS_NETWORK_LINK_STATS
#define ETHARP_STATS                    OS_NETWORK_ETHARP_STATS
#define IP_STATS                        OS_NETWORK_IP_STATS
#define IPFRAG_STATS                    OS_NETWORK_IPFRAG_STATS
#define ICMP_STATS                      OS_NETWORK_ICMP_STATS
#define IGMP_STATS                      OS_NETWORK_IGMP_STATS
#define UDP_STATS                       OS_NETWORK_UDP_STATS
#define TCP_STATS                       OS_NETWORK_TCP_STATS
#define MEM_STATS                       OS_NETWORK_MEM_STATS
#define MEMP_STATS                      OS_NETWORK_MEMP_STATS
#define SYS_STATS                       OS_NETWORK_SYS_STATS

#else

#define LINK_STATS                      0
#define IP_STATS                        0
#define IPFRAG_STATS                    0
#define ICMP_STATS                      0
#define IGMP_STATS                      0
#define UDP_STATS                       0
#define TCP_STATS                       0
#define MEM_STATS                       0
#define MEMP_STATS                      0
#define SYS_STATS                       0
#define LWIP_STATS_DISPLAY              0

#endif //(LWIP_STATS)
#define LWIP_PROVIDE_ERRNO              OS_NETWORK_LWIP_PROVIDE_ERRNO
//PPP options
#define PPP_SUPPORT                     OS_NETWORK_PPP_SUPPORT
#define PPPOE_SUPPORT                   OS_NETWORK_PPPOE_SUPPORT
#define PPPOS_SUPPORT                   OS_NETWORK_PPPOS_SUPPORT

//#if (PPP_SUPPORT)

#define NUM_PPP                         OS_NETWORK_NUM_PPP
#define PAP_SUPPORT                     OS_NETWORK_PAP_SUPPORT
#define CHAP_SUPPORT                    OS_NETWORK_CHAP_SUPPORT
#define MSCHAP_SUPPORT                  OS_NETWORK_MSCHAP_SUPPORT
#define CBCP_SUPPORT                    OS_NETWORK_CBCP_SUPPORT
#define CCP_SUPPORT                     OS_NETWORK_CCP_SUPPORT
#define VJ_SUPPORT                      OS_NETWORK_VJ_SUPPORT
#define MD5_SUPPORT                     OS_NETWORK_MD5_SUPPORT
/*
 * Timeouts
 */
#define FSM_DEFTIMEOUT                  OS_NETWORK_FSM_DEFTIMEOUT
#define FSM_DEFMAXTERMREQS              OS_NETWORK_FSM_DEFMAXTERMREQS
#define FSM_DEFMAXCONFREQS              OS_NETWORK_FSM_DEFMAXCONFREQS
#define FSM_DEFMAXNAKLOOPS              OS_NETWORK_FSM_DEFMAXNAKLOOPS
#define UPAP_DEFTIMEOUT                 OS_NETWORK_UPAP_DEFTIMEOUT
#define UPAP_DEFREQTIME                 OS_NETWORK_UPAP_DEFREQTIME
#define CHAP_DEFTIMEOUT                 OS_NETWORK_CHAP_DEFTIMEOUT
#define CHAP_DEFTRANSMITS               OS_NETWORK_CHAP_DEFTRANSMITS
#define LCP_ECHOINTERVAL                OS_NETWORK_LCP_ECHOINTERVAL
#define LCP_MAXECHOFAILS                OS_NETWORK_LCP_MAXECHOFAILS
#define PPP_MAXIDLEFLAG                 OS_NETWORK_PPP_MAXIDLEFLAG
/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define PPP_MTU                         OS_NETWORK_PPP_MTU
#define PPP_MAXMTU                      OS_NETWORK_PPP_MAXMTU
#define PPP_MINMTU                      OS_NETWORK_PPP_MINMTU
#define PPP_MRU                         OS_NETWORK_PPP_MRU
#define PPP_MAXMRU                      OS_NETWORK_PPP_MAXMRU
#define PPP_DEFMRU                      OS_NETWORK_PPP_DEFMRU
#define PPP_MINMRU                      OS_NETWORK_PPP_MINMRU

#define MAXNAMELEN                      OS_NETWORK_MAXNAMELEN
#define MAXSECRETLEN                    OS_NETWORK_MAXSECRETLEN

//#endif //(PPP_SUPPORT)

//Checksum options
#define CHECKSUM_GEN_IP                 OS_NETWORK_CHECKSUM_GEN_IP
#define CHECKSUM_GEN_UDP                OS_NETWORK_CHECKSUM_GEN_UDP
#define CHECKSUM_GEN_TCP                OS_NETWORK_CHECKSUM_GEN_TCP
#define CHECKSUM_GEN_ICMP               OS_NETWORK_CHECKSUM_GEN_ICMP
#define CHECKSUM_CHECK_IP               OS_NETWORK_CHECKSUM_CHECK_IP
#define CHECKSUM_CHECK_UDP              OS_NETWORK_CHECKSUM_CHECK_UDP
#define CHECKSUM_CHECK_TCP              OS_NETWORK_CHECKSUM_CHECK_TCP
#define LWIP_CHECKSUM_ON_COPY           OS_NETWORK_CHECKSUM_ON_COPY
//Debugging options
#define LWIP_DEBUG                      OS_NETWORK_DEBUG
#define LWIP_DBG_MIN_LEVEL              OS_NETWORK_DEBUG_MIN_LEVEL
#define LWIP_DBG_TYPES_ON               OS_NETWORK_DEBUG_TYPES_ON
#define ETHARP_DEBUG                    OS_NETWORK_DEBUG_ETHARP
#define NETIF_DEBUG                     OS_NETWORK_DEBUG_NETIF
#define PBUF_DEBUG                      OS_NETWORK_DEBUG_PBUF
#define API_LIB_DEBUG                   OS_NETWORK_DEBUG_API_LIB
#define API_MSG_DEBUG                   OS_NETWORK_DEBUG_API_MSG
#define SOCKETS_DEBUG                   OS_NETWORK_DEBUG_SOCKETS
#define ICMP_DEBUG                      OS_NETWORK_DEBUG_ICMP
#define IGMP_DEBUG                      OS_NETWORK_DEBUG_IGMP
#define INET_DEBUG                      OS_NETWORK_DEBUG_INET
#define IP_DEBUG                        OS_NETWORK_DEBUG_IP
#define IP_REASS_DEBUG                  OS_NETWORK_DEBUG_IP_REASS
#define RAW_DEBUG                       OS_NETWORK_DEBUG_RAW
#define MEM_DEBUG                       OS_NETWORK_DEBUG_MEM
#define MEMP_DEBUG                      OS_NETWORK_DEBUG_MEMP
#define SYS_DEBUG                       OS_NETWORK_DEBUG_SYS
#define TIMERS_DEBUG                    OS_NETWORK_DEBUG_TIMERS
#define TCP_DEBUG                       OS_NETWORK_DEBUG_TCP
#define TCP_INPUT_DEBUG                 OS_NETWORK_DEBUG_TCP_INPUT
#define TCP_FR_DEBUG                    OS_NETWORK_DEBUG_TCP_FR
#define TCP_RTO_DEBUG                   OS_NETWORK_DEBUG_TCP_RTO
#define TCP_CWND_DEBUG                  OS_NETWORK_DEBUG_TCP_CWND
#define TCP_WND_DEBUG                   OS_NETWORK_DEBUG_TCP_WND
#define TCP_OUTPUT_DEBUG                OS_NETWORK_DEBUG_TCP_OUTPUT
#define TCP_RST_DEBUG                   OS_NETWORK_DEBUG_TCP_RST
#define TCP_QLEN_DEBUG                  OS_NETWORK_DEBUG_TCP_QLEN
#define UDP_DEBUG                       OS_NETWORK_DEBUG_UDP
#define TCPIP_DEBUG                     OS_NETWORK_DEBUG_TCPIP
#define PPP_DEBUG                       OS_NETWORK_DEBUG_PPP
#define SLIP_DEBUG                      OS_NETWORK_DEBUG_SLIP
#define DHCP_DEBUG                      OS_NETWORK_DEBUG_DHCP
#define AUTOIP_DEBUG                    OS_NETWORK_DEBUG_AUTOIP
#define SNMP_MSG_DEBUG                  OS_NETWORK_DEBUG_SNMP_MSG
#define SNMP_MIB_DEBUG                  OS_NETWORK_DEBUG_SNMP_MIB
#define DNS_DEBUG                       OS_NETWORK_DEBUG_DNS

#endif /* __LWIPOPTS_H__ */
