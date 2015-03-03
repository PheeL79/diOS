/**************************************************************************//**
* @file    os_config_tasks_prio.h
* @brief   Config header file for OS tasks priorities.
* @author  A. Filyanov
******************************************************************************/
#ifndef _OS_CONFIG_TASKS_PRIO_H_
#define _OS_CONFIG_TASKS_PRIO_H_

//------------------------------------------------------------------------------
// runtime (initial) priority
#define OS_TASK_PRIO_LOG                    (20)
#define OS_TASK_PRIO_FS                     (90)
#define OS_TASK_PRIO_USB                    (100)
#define OS_TASK_PRIO_NET                    (120)
#define OS_TASK_PRIO_AUDIO                  (150)
#define OS_TASK_PRIO_SHELL                  (10)

// power priority
#define OS_TASK_PRIO_PWR_LOG                (OS_PWR_PRIO_MAX - 20)
#define OS_TASK_PRIO_PWR_FS                 (OS_PWR_PRIO_MAX - 10)
#define OS_TASK_PRIO_PWR_USB                (OS_PWR_PRIO_MAX - 5)
#define OS_TASK_PRIO_PWR_NET                (OS_PWR_PRIO_MAX - 5)
#define OS_TASK_PRIO_PWR_AUDIO              (OS_PWR_PRIO_MAX - 10)
#define OS_TASK_PRIO_PWR_SHELL              (OS_PWR_PRIO_MAX - 30)

#endif // _OS_CONFIG_TASKS_PRIO_H_