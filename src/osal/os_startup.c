/***************************************************************************//**
* @file    os_startup.c
* @brief   OS Startup.
* @author  A. Filyanov
*******************************************************************************/
#include "os_task.h"
#include "os_memory.h"
#include "os_startup.h"

/******************************************************************************/
static U32 StartupTasksCountGet(OS_TaskConfig** os_startup_v_pp);
U32 StartupTasksCountGet(OS_TaskConfig** os_startup_v_pp)
{
U32 tasks_count = 0;
    while (OS_NULL != *os_startup_v_pp++) {
        ++tasks_count;
    }
    return tasks_count;
}

/******************************************************************************/
Status OS_StartupApplication(void)
{
extern const OS_TaskConfig* os_startup_v[];
OS_TaskConfig** task_cfg_pp = (OS_TaskConfig**)&os_startup_v[0];
OS_TaskHd* startup_tasks_ready_v_p = OS_Malloc(StartupTasksCountGet(task_cfg_pp) * sizeof(OS_TaskHd));
OS_TaskHd* startup_tasks_ready_tmp_v_p = startup_tasks_ready_v_p;
Status s = S_OK;
    OS_LOG(D_INFO, "OS startup run...");
    if (OS_NULL == startup_tasks_ready_v_p) { return S_NO_MEMORY; }
    if (OS_NULL != task_cfg_pp) {
        // Create and suspend startup tasks.
        while (OS_NULL != *task_cfg_pp) {
            OS_TaskHd thd = OS_NULL;
            OS_TaskConfig* task_cfg_p = *task_cfg_pp;
            IF_STATUS_OK(s = task_cfg_p->func_power(task_cfg_p->args_p, PWR_STARTUP)) {
                IF_STATUS(s = OS_TaskCreate(task_cfg_p, &thd)) {
                    OS_ASSERT(OS_FALSE);
                }
                OS_TaskSuspend(thd);
            }
            *startup_tasks_ready_tmp_v_p++ = thd;
            ++task_cfg_pp;
        }
        // Run tasks.
        task_cfg_pp = (OS_TaskConfig**)&os_startup_v[0];
        startup_tasks_ready_tmp_v_p = startup_tasks_ready_v_p;
        while (OS_NULL != *task_cfg_pp++) {
            OS_TaskResume(*startup_tasks_ready_tmp_v_p++);
        }
    }
//error:
    OS_Free(startup_tasks_ready_v_p);
    return s;
}

/******************************************************************************/
Status OS_StartupSystem(void)
{
extern const OS_TaskConfig task_sv_cfg, task_shell_cfg;
const OS_PowerState power = OS_PowerStateGet();
Status s;
    IF_STATUS(s = task_sv_cfg.func_power(task_sv_cfg.args_p, power))    { goto error; }
    IF_STATUS(s = task_shell_cfg.func_power(task_sv_cfg.args_p, power)) { goto error; }
error:
    return s;
}