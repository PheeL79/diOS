/***************************************************************************//**
* @file    os_task_net.h
* @brief   Network daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_NET_H_
#define _OS_TASK_NET_H_

#include "os_config.h"

#if (OS_NETWORK_ENABLED)
#define OS_DAEMON_NAME_NET          "NetD"
#endif //(OS_NETWORK_ENABLED)

#endif //_OS_TASK_NET_H_