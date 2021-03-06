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
#define OS_STDIN_LEN    4

enum {
    OS_TASK_ATTR_SINGLE,
    OS_TASK_ATTR_RECREATE,
//    OS_TASK_ATTR_MPU,
//    OS_TASK_ATTR_FPU,
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

typedef U8 OS_TaskPrio;

typedef OS_Owner    OS_TaskHd;      //Task handle
typedef TaskId      OS_TaskId;      //Task identifier
typedef void        OS_TaskStorage; //Task storage

typedef struct {
    const void*     args_p;
    OS_TaskStorage* stor_p;
} OS_TaskArgs;

//typedef struct {
//    OS_TaskPrio     curr_prio;
//    U16             stack_remain;
//    U8              cpu_load;
//} OS_TaskStat;

typedef TaskStatus_t OS_TaskStats;

typedef struct {
    ConstStr            name[OS_TASK_NAME_LEN];
    void                (*func_main)(OS_TaskArgs*);
    Status              (*func_power)(OS_TaskArgs*, const OS_PowerState);
    void*               args_p;
    OS_TaskAttrs        attrs;
    OS_TaskPrio         prio_init;
    OS_PowerPrio        prio_power;
    U8                  timeout;
    U16                 storage_size;
    U16                 stack_size;
    U8                  stdin_len;
} OS_TaskConfig;

//------------------------------------------------------------------------------
/// @brief      Init task.
/// @param[in]  args_p          Task arguments.
/// @return     #Status.
WEAK static Status  OS_TaskInit(OS_TaskArgs* args_p);

/// @brief      Task main function.
/// @param[in]  args_p          Task arguments.
/// @return     None.
WEAK static void    OS_TaskMain(OS_TaskArgs* args_p);

/// @brief      Task main function.
/// @param[in]  args_p          Task arguments.
/// @param[in]  state           Task new power state.
/// @return     #Status.
WEAK static Status  OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state);

#include "os_queue.h" //deps on above declarations
//------------------------------------------------------------------------------
/// @brief      Create a task.
/// @param[in]  args_p          Task arguments.
/// @param[in]  cfg_p           Task config.
/// @param[out] thd_p           Task handle.
/// @return     #Status.
Status          OS_TaskCreate(const void* args_p, const OS_TaskConfig* cfg_p, OS_TaskHd* thd_p);

/// @brief      Delete the task.
/// @param[in]  thd             Task handle.
/// @return     #Status.
Status          OS_TaskDelete(const OS_TaskHd thd);

/// @brief      Delay the task.
/// @param[in]  timeout         Delay timeout.
/// @return     None.
void            OS_TaskDelay(const OS_TimeMs timeout);

/// @brief      Delay the task until.
/// @param[in]  tick_last_p     Last time task unblocked system ticks.
/// @param[in]  timeout         Delay timeout.
/// @return     None.
void            OS_TaskDelayUntil(OS_Tick* tick_last_p, const OS_TimeMs timeout);

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

/// @brief      Connect two tasks by each other.
/// @param[in]  signal_thd      Task handle.
/// @param[in]  slot_thd        Task handle.
/// @return     #Status.
Status          OS_TasksConnect(const OS_TaskHd signal_thd, const OS_TaskHd slot_thd);

/// @brief      Disconnect tasks.
/// @param[in]  signal_thd      Task handle.
/// @param[in]  slot_thd        Task handle.
/// @return     #Status.
Status          OS_TasksDisconnect(const OS_TaskHd signal_thd, const OS_TaskHd slot_thd);

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
ConstStrP       OS_TaskStateNameGet(const OS_TaskState state);

/// @brief      Get task name.
/// @param[in]  thd             Task handle.
/// @return     Name.
ConstStrP       OS_TaskNameGet(const OS_TaskHd thd);

/// @brief      Get task attributes.
/// @param[in]  thd             Task handle.
/// @return     Attributes.
OS_TaskAttrs    OS_TaskAttrsGet(const OS_TaskHd thd);

/// @brief      Get task configuration.
/// @param[in]  thd             Task handle.
/// @return     Task configuration.
const OS_TaskConfig* OS_TaskConfigGet(const OS_TaskHd thd);

/// @brief      Get task arguments.
/// @param[in]  thd             Task handle.
/// @return     Task arguments.
const void*     OS_TaskArgumentsGet(const OS_TaskHd thd);

/// @brief      Get task storage.
/// @param[in]  thd             Task handle.
/// @return     Task storage.
OS_TaskStorage* OS_TaskStorageGet(const OS_TaskHd thd);

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
OS_TaskHd       OS_TaskByNameGet(ConstStrP name_p);

/// @brief      Get the next task.
/// @param[in]  thd             Task handle.
/// @return     Task handle.
OS_TaskHd       OS_TaskNextGet(const OS_TaskHd thd);

/// @brief      Get the system supervisor task standart input queue.
/// @return     Queue handle.
OS_QueueHd      OS_TaskSvStdInGet(void);

/// @brief      Get the task standart input/output queue.
/// @param[in]  thd             Task handle.
/// @return     Queue handle.
OS_QueueHd      OS_TaskStdInGet(const OS_TaskHd thd);

/// @brief      Set the task standart input/output queue.
/// @param[in]  thd             Task handle.
/// @param[in]  qhd             Queue handle.
/// @return     Queue handle.
//Status          OS_TaskStdInSet(const OS_TaskHd thd, const OS_QueueHd qhd);

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
