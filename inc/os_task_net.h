/***************************************************************************//**
* @file    os_task_net.h
* @brief   Network daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_NET_H_
#define _OS_TASK_NET_H_

#include "os_config.h"

#if (HAL_ETH_ENABLED)
#define OS_DAEMON_NAME_NET          "NetD"
//enum {
//    OS_TIM_ID_ETH0_LINK_STATUS_REFRESH = OS_TIM_ID_APP
//};
#endif //(HAL_ETH_ENABLED)

#endif //_OS_TASK_NET_H_