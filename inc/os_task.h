/***************************************************************************//**
* @file    os_task.h
* @brief   OS Task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_H_
#define _OS_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "status.h"
#include "os_config.h"
#include "os_common.h"
#include "os_power.h"
#include "os_time.h"

/**
* \defgroup OS_Task OS_Task
* @{
*/
//------------------------------------------------------------------------------
/// @brief Common declarations.

#define OS_THIS_TASK    OS_NULL

#define OS_STDIO_LEN    4
enum {
    OS_STDIO_IN,
    OS_STDIO_OUT
};
typedef U8 OS_StdIoDir;

enum {
    OS_TASK_ATTR_UNDEF,
    OS_TASK_ATTR_RECREATE,
    OS_TASK_ATTR_LAST
};
typedef U8 OS_TaskAttrs;

enum {
    OS_TASK_STATE_UNDEF,
    OS_TASK_STATE_READY,
    OS_TASK_STATE_RUN,
    OS_TASK_STATE_BLOCK,
    OS_TASK_STATE_SUSPEND,
    OS_TASK_STATE_DELETED,
    OS_TASK_STATE_LAST
};
typedef U8 OS_TaskState;

enum {
    OS_TASK_PRIO_UNDEF,
    OS_TASK_PRIO_LOW            = OS_PRIORITY_MIN,
    OS_TASK_PRIO_BELOW_NORMAL,
    OS_TASK_PRIO_NORMAL,
    OS_TASK_PRIO_ABOVE_NORMAL,
    OS_TASK_PRIO_HIGH,
    OS_TASK_PRIO_REALTIME,
    OS_TASK_PRIO_MAX            = OS_PRIORITY_MAX - 1,
    OS_TASK_PRIO_LAST           = OS_TASK_PRIO_MAX
};
typedef U8 OS_TaskPrio;

typedef OS_Owner    OS_TaskHd;  //Task handle
typedef U8          OS_TaskId;  //Task identifier
typedef void        OS_TaskArgs;//Task arguments

//typedef struct {
//    OS_TaskPrio     curr_prio;
//    U16             stack_remain;
//    U8              cpu_load;
//} OS_TaskStat;

typedef TaskStatus_t OS_TaskStats;

typedef struct {
    ConstStr            name[OS_TASK_NAME_LEN];
    void                (*func_main)(void*);
    Status              (*func_power)(void*, const OS_PowerState);
    void*               args_p;
    OS_TaskAttrs        attrs;
    OS_TaskPrio         prio_init;
    OS_PowerPrio        prio_power;
    U8                  timeout;
    U16                 stack_size;
    U8                  stdin_len;
    U8                  stdout_len;
} OS_TaskConfig;

//------------------------------------------------------------------------------
/// @brief      Init task.
/// @param[in]  args_p          Task arguments.
/// @return     #Status.
static Status   OS_TaskInit(OS_TaskArgs* args_p);

/// @brief      Task main function.
/// @param[in]  args_p          Task arguments.
/// @return     None.
static void     OS_TaskMain(OS_TaskArgs* args_p);

/// @brief      Task main function.
/// @param[in]  args_p          Task arguments.
/// @param[in]  state           Task new power state.
/// @return     #Status.
static Status   OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state);

#include "os_queue.h" //deps on above declarations
//------------------------------------------------------------------------------
/// @brief      Create a task.
/// @param[in]  cfg_p           Task config.
/// @param[out] thd_p           Task handle.
/// @return     #Status.
Status          OS_TaskCreate(const OS_TaskConfig* cfg_p, OS_TaskHd* thd_p);

/// @brief      Delete the task.
/// @param[in]  thd             Task handle.
/// @return     #Status.
Status          OS_TaskDelete(const OS_TaskHd thd);

/// @brief      Delay the task.
/// @param[in]  timeout         Delay timeout.
/// @return     None.
void            OS_TaskDelay(const TimeMs timeout);

/// @brief      Delay the task until.
/// @param[in]  tick_last_p     Last time task unblocked system ticks.
/// @param[in]  timeout         Delay timeout.
/// @return     None.
void            OS_TaskDelayUntil(OS_Tick* tick_last_p, const TimeMs timeout);

/// @brief      Suspend the task.
/// @param[in]  thd             Task handle.
/// @return     None.
void            OS_TaskSuspend(const OS_TaskHd thd);

/// @brief      Resume the task.
/// @param[in]  thd             Task handle.
/// @return     None.
void            OS_TaskResume(const OS_TaskHd thd);

/// @brief      Get task id.
/// @param[in]  thd             Task handle.
/// @return     Task id.
OS_TaskId       OS_TaskIdGet(const OS_TaskHd thd);

/// @brief      Get current task handle.
/// @param[in]  thd             Task handle.
/// @return     Task handle.
OS_TaskHd       OS_TaskGet(void);

/// @brief      Get task handle by it's id.
/// @param[in]  tid             Task id.
/// @return     Task handle.
OS_TaskHd       OS_TaskByIdGet(const OS_TaskId tid);

/// @brief      Get current task parent's handle.
/// @return     Task handle.
OS_TaskHd       OS_TaskParentGet(void);

/// @brief      Get parent's task by task handle.
/// @param[in]  thd             Task handle.
/// @return     Task handle.
OS_TaskHd       OS_TaskParentByHdGet(const OS_TaskHd thd);

/// @brief      Get tasks count.
/// @return     Tasks count.
U32             OS_TasksCountGet(void);

/// @brief      Get tasks statistics.
/// @param[out] stats_p         Task statistics.
/// @param[in]  stats_count     Task statistics count.
/// @param[out] uptime_p        System uptime.
/// @return     Task statistics count that were populated.
U32             OS_TasksStatsGet(OS_TaskStats* stats_p, const U32 stats_count, U32* uptime_p);

/// @brief      Get task state.
/// @param[in]  thd             Task handle.
/// @return     Task state.
OS_TaskState    OS_TaskStateGet(const OS_TaskHd thd);

/// @brief      Get task state name.
/// @param[in]  state           Task state.
/// @return     Name.
ConstStrPtr     OS_TaskStateNameGet(const OS_TaskState state);

/// @brief      Get task name.
/// @param[in]  thd             Task handle.
/// @return     Name.
ConstStrPtr     OS_TaskNameGet(const OS_TaskHd thd);

/// @brief      Get task attributes.
/// @param[in]  thd             Task handle.
/// @return     Attributes.
OS_TaskAttrs    OS_TaskAttrsGet(const OS_TaskHd thd);

/// @brief      Get task configuration.
/// @param[in]  thd             Task handle.
/// @return     Task configuration.
const OS_TaskConfig* OS_TaskConfigGet(const OS_TaskHd thd);

/// @brief      Get task storage.
/// @param[in]  thd             Task handle.
/// @return     Task storage.
void*           OS_TaskStorageGet(const OS_TaskHd thd);

/// @brief      Get task power state.
/// @param[in]  thd             Task handle.
/// @return     Task power state.
OS_PowerState   OS_TaskPowerStateGet(const OS_TaskHd thd);

/// @brief      Get task priority.
/// @param[in]  thd             Task handle.
/// @return     Task priority.
OS_TaskPrio     OS_TaskPriorityGet(const OS_TaskHd thd);

/// @brief      Set task priority.
/// @param[in]  thd             Task handle.
/// @param[in]  prio            Priority.
/// @return     #Status.
Status          OS_TaskPrioritySet(const OS_TaskHd thd, const OS_TaskPrio prio);

/// @brief      Get task by it's name.
/// @param[in]  name_p          Task name.
/// @return     Task handle.
OS_TaskHd       OS_TaskByNameGet(ConstStrPtr name_p);

/// @brief      Get the next task.
/// @param[in]  thd             Task handle.
/// @return     Task handle.
OS_TaskHd       OS_TaskNextGet(const OS_TaskHd thd);

/// @brief      Get the system supervisor task standart input queue.
/// @return     Queue handle.
OS_QueueHd      OS_TaskSvcStdInGet(void);

/// @brief      Get the task standart input/output queue.
/// @param[in]  thd             Task handle.
/// @param[in]  dir             I\O direction.
/// @return     Queue handle.
OS_QueueHd      OS_TaskStdIoGet(const OS_TaskHd thd, const OS_StdIoDir dir);

/// @brief      Set the task standart input/output queue.
/// @param[in]  thd             Task handle.
/// @param[in]  qhd             Queue handle.
/// @param[in]  dir             I\O direction.
/// @return     Queue handle.
//Status          OS_TaskStdIoSet(const OS_TaskHd thd, const OS_QueueHd qhd, const OS_StdIoDir dir);

/**
* \addtogroup OS_MPU_Task MPU specific functions.
* @{
*/
//------------------------------------------------------------------------------
//Status            OS_MPU_TaskCreate(const TaskConfig* cfg_p);

/**@}*/ //OS_MPU_Task

/**
* \addtogroup OS_ISR_Task ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------

/**@}*/ //OS_ISR_Task

/**@}*/ //OS_Task

#ifdef __cplusplus
}
#endif

#endif // _OS_TASK_H_
