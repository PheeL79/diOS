/**************************************************************************//**
* @file    os_config_prio.h
* @brief   Config header file for OS tasks priorities.
* @author  A. Filyanov
******************************************************************************/
#ifndef _OS_CONFIG_PRIO_H_
#define _OS_CONFIG_PRIO_H_

//------------------------------------------------------------------------------
// runtime (initial) priority
#define OS_PRIO_TASK_TIMERS                 (2)
#define OS_PRIO_TASK_LOG                    (20)
#define OS_PRIO_TASK_FS                     (90)
#define OS_PRIO_TASK_USB                    (100)
#define OS_PRIO_TASK_NET                    (120)
#define OS_PRIO_TASK_AUDIO                  (150)
#define OS_PRIO_TASK_SHELL                  (10)
//OS Network daemons
#define OS_PRIO_TASK_TCPIP                  (190)
#define OS_PRIO_TASK_SLIP                   (190)
#define OS_PRIO_TASK_PPP_INPUT              (190)
#define OS_PRIO_TASK_LWIP                   (190)

// power priority
#define OS_PRIO_PWR_TASK_LOG                (OS_PWR_PRIO_MAX - 20)
#define OS_PRIO_PWR_TASK_FS                 (OS_PWR_PRIO_MAX - 10)
#define OS_PRIO_PWR_TASK_USB                (OS_PWR_PRIO_MAX - 5)
#define OS_PRIO_PWR_TASK_NET                (OS_PWR_PRIO_MAX - 5)
#define OS_PRIO_PWR_TASK_AUDIO              (OS_PWR_PRIO_MAX - 10)
#define OS_PRIO_PWR_TASK_SHELL              (OS_PWR_PRIO_MAX - 30)

#endif //_OS_CONFIG_PRIO_H_