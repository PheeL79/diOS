/***************************************************************************//**
* @file    task_sv.c
* @brief   Supervisor task.
* @author  A. Filyanov
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "hal.h"
#include "osal.h"
#include "os_supervise.h"
#include "os_memory.h"
#include "os_file_system.h"
#include "os_environment.h"
#include "os_signal.h"
#include "os_message.h"
#include "os_debug.h"
#include "os_driver.h"
#include "os_shell.h"
#include "os_startup.h"

//------------------------------------------------------------------------------
#define MDL_NAME            "task_sv"

//------------------------------------------------------------------------------
//Task arguments
typedef struct {
//    OS_DriverHd drv_power;
    OS_DriverHd drv_led_pulse;
} TaskArgs;

static TaskArgs task_args = {
//    .drv_power      = OS_NULL,
    .drv_led_pulse  = OS_NULL,
};
volatile OS_QueueHd sv_stdin_qhd;
static volatile OS_TaskHd deadlock_thd;

//------------------------------------------------------------------------------
static void     MessagesHandler(TaskArgs* task_args_p);
static Status   SystemPowerStateSet(TaskArgs* task_args_p, const OS_PowerState state);
static Status   SystemPowerStateForTasksSet(const OS_PowerState state);
static Status   SystemPowerStateForDriversSet(const OS_PowerState state);
static Status   TaskDeadLockTest(void);
static Status   TaskDeadLockAction(void);
static void     Reboot(TaskArgs* task_args_p);
static void     Shutdown(TaskArgs* task_args_p);

//------------------------------------------------------------------------------
const OS_TaskConfig task_sv_cfg = {
    .name       = "Sv",
    .func_main  = OS_TaskMain,
    .func_power = OS_TaskPower,
    .args_p     = (void*)&task_args,
    .prio_init  = OS_TASK_PRIO_MAX,
    .prio_power = OS_PWR_PRIO_MAX,
    .stack_size = OS_STACK_SIZE_MIN * 2,
    .stdin_len  = OS_STDIN_LEN,
};

/******************************************************************************/
Status OS_TaskInit(OS_TaskArgs* args_p)
{
Status s;
    HAL_LOG(D_INFO, "Init");
//    {
//        const OS_DriverConfig drv_cfg = {
//            .name       = "POWER",
//            .itf_p      = drv_power_v[DRV_ID_POWER],
//            .mode_io    = DRV_MODE_IO_DEFAULT,
//            .prio_power = OS_PWR_PRIO_MAX
//        };
//        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args.drv_power)) { return s; }
//        IF_STATUS(s = OS_DriverInit(task_args.drv_power)) { return s; }
//        IF_STATUS(s = OS_DriverOpen(task_args.drv_power, OS_NULL)) { return s; }
//    }

    {
        const OS_DriverConfig drv_cfg = {
            .name       = "LED_PLS",
            .itf_p      = drv_led_v[DRV_ID_LED_PULSE],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&task_args.drv_led_pulse)) { return s; }
        IF_STATUS(s = OS_DriverInit(task_args.drv_led_pulse, OS_NULL)) { return s; }
        IF_STATUS(s = OS_DriverOpen(task_args.drv_led_pulse, OS_NULL)) { return s; }
    }
    OS_ASSERT(S_OK == (s = OS_TaskCreate(&task_sv_cfg, OS_NULL)));
    return s;
}

/******************************************************************************/
void OS_TaskMain(OS_TaskArgs* args_p)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
    sv_stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
    OS_ASSERT(S_OK == OS_StartupApplication());
    OS_ASSERT(S_OK == OS_StartupDeInit());
#if (1 == TIMER_IWDG_ENABLED)
    OS_ASSERT(S_OK == TIMER_IWDG_Start());
#endif // TIMER_IWDG_ENABLED
	for(;;) {
        MessagesHandler(task_args_p);
    }
}

/******************************************************************************/
Status OS_TaskPower(OS_TaskArgs* args_p, const OS_PowerState state)
{
TaskArgs* task_args_p = (TaskArgs*)args_p;
const OS_PowerState state_prev = OS_PowerStateGet();
Status s = S_OK;
    if (PWR_STARTUP == state) {
        IF_STATUS(s = OS_TaskInit((OS_TaskArgs*)task_args_p)) {
            return s;
        }
    } else {
        IF_STATUS(s = SystemPowerStateSet(task_args_p, state)) {
            OS_LOG(D_INFO, "Attempt to rollback power state: %s", OS_PowerStateNameGet(state_prev));
            IF_STATUS(s = SystemPowerStateSet(task_args_p, state_prev)) {
            }
            return s;
        }
    }
    return s;
}

/******************************************************************************/
Status SystemPowerStateSet(TaskArgs* task_args_p, const OS_PowerState state)
{
//const HAL_DriverItf* drv_power_itf_p = OS_DriverItfGet(task_args_p->drv_power);
HAL_DriverItf* drv_power_itf_p = drv_power_v[DRV_ID_POWER];
extern volatile OS_Env os_env;
Status s;
    if ((PWR_ON == state) || (PWR_STARTUP == state)) {
        //Direct call to IoCtl func bypass OS_Driver interface.
        //(no API func calls are allowed with the suspended scheduler).
        IF_STATUS_OK(s = drv_power_itf_p->IoCtl(DRV_REQ_STD_POWER_SET, (void*)&state)) {
            IF_STATUS_OK(s = SystemPowerStateForDriversSet(state)) {
                IF_STATUS_OK(s = SystemPowerStateForTasksSet(state)) {
                    os_env.hal_env_p->power = state;
                    OS_LOG(D_INFO, "Power state: %s", OS_PowerStateNameGet(os_env.hal_env_p->power));
                }
            }
        } else { OS_LOG_S(D_WARNING, s); }
    } else {
        IF_STATUS_OK(s = SystemPowerStateForTasksSet(state)) {
            IF_STATUS_OK(s = SystemPowerStateForDriversSet(state)) {
                os_env.hal_env_p->power = state;
                OS_LOG(D_INFO, "Power state: %s", OS_PowerStateNameGet(os_env.hal_env_p->power));
                //Direct call to IoCtl func bypass OS_Driver interface.
                //(no API func calls are allowed with the suspended scheduler).
                IF_STATUS(s = drv_power_itf_p->IoCtl(DRV_REQ_STD_POWER_SET, (void*)&state)) {
                    OS_LOG_S(D_WARNING, s);
                }
            }
        }
    }
    return s;
}

/******************************************************************************/
Status SystemPowerStateForTasksSet(const OS_PowerState state)
{
extern Status OS_TaskPowerStateSet(const OS_TaskHd thd, const OS_PowerState state);
extern void   OS_TaskPowerPrioritySort(const SortDirection sort_dir);
const OS_Signal signal = OS_SignalCreate(OS_SIG_PWR, state);
const OS_TaskHd sv_thd = OS_TaskGet();
OS_TaskHd thd;
OS_TaskHd next_thd = OS_NULL;
Status s = S_OK; //!
    OS_LOG(D_DEBUG, "Set power state for the tasks");
    const SortDirection sort_dir = (PWR_ON == state) ? SORT_DESCENDING : SORT_ASCENDING;
    OS_TaskPowerPrioritySort(sort_dir);
    thd = OS_TaskNextGet(next_thd); //first task in the list.
    while (OS_NULL != thd) {
        const OS_TaskHd par_thd = OS_TaskParentByHdGet(thd);
        next_thd = OS_TaskNextGet(thd);
        if ((par_thd != OS_NULL) && (sv_thd != thd)) { //ignore OS system tasks.
            const OS_QueueHd stdin_qhd = OS_TaskStdInGet(thd);
            if (OS_NULL != stdin_qhd) {
                IF_STATUS_OK(s = OS_SignalSend(stdin_qhd, signal, OS_MSG_PRIO_HIGH)) {
                    OS_Message* msg_p;
                    IF_STATUS_OK(s = OS_MessageReceive(sv_stdin_qhd, &msg_p, OS_TIMEOUT_POWER)) {
                        if (OS_SignalIs(msg_p)) {
                            switch (OS_SignalIdGet(msg_p)) {
                                case OS_SIG_PWR_ACK:
                                    IF_STATUS_OK(s = (Status)OS_SignalDataGet(msg_p)) {
                                        if (PWR_SHUTDOWN == state) {
                                            IF_STATUS(s = OS_TaskDelete(thd)) {
                                                //TODO(A.Filyanov) Status handler!
                                            }
                                        }
//                                        } else {
//                                            if ((PWR_ON == state) || (PWR_STARTUP == state)) {
//                                                OS_TaskResume(thd);
//                                            } else {
//                                                OS_TaskSuspend(thd);
//                                            }
//                                        }
                                    } else {
                                        return s;
                                    }
                                    break;
                                default:
                                    OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                                    break;
                            }
                        } else {
                            OS_MessageDelete(msg_p); // free message allocated memory
                        }
                    } else {} //TODO(A.Filyanov) Status handler!
                } else {} //TODO(A.Filyanov) Status handler!
            }
        }
        thd = next_thd;
    }
    return s;
}

/******************************************************************************/
Status SystemPowerStateForDriversSet(const OS_PowerState state)
{
extern void OS_DriverPowerPrioritySort(const SortDirection sort_dir);
OS_DriverHd dhd;
Status s;
    OS_LOG(D_DEBUG, "Set power state for the drivers");
    const SortDirection sort_dir = (PWR_ON == state) ? SORT_DESCENDING : SORT_ASCENDING;
    OS_DriverPowerPrioritySort(sort_dir);
    dhd = OS_DriverNextGet(OS_NULL); //first driver in the list.
    while (OS_NULL != dhd) {
        s = OS_DriverIoCtl(dhd, DRV_REQ_STD_POWER_SET, (void*)&state);
        if (S_OK == s) {
        } else if (S_ISNT_OPENED == s) { //ignore
        } else if (S_UNDEF_FUNCTION == s) { //ignore absent IoCtl()
            OS_LOG_S(D_DEBUG, s);
        } else {
            OS_LOG(D_WARNING, "%s: power state set failed!", OS_DriverNameGet(dhd));
            return s;
        }
        dhd = OS_DriverNextGet(dhd);
    }
    return S_OK;
}

/******************************************************************************/
void MessagesHandler(TaskArgs* task_args_p)
{
extern Status OS_TaskTimeoutReset(const OS_TaskHd thd);
extern VBL is_idle;
static State led_state = OFF;
OS_Message* msg_p;

    IF_STATUS_OK(OS_MessageReceive(sv_stdin_qhd, &msg_p, OS_PULSE_RATE)) {
        if (OS_SignalIs(msg_p)) {
            switch (OS_SignalIdGet(msg_p)) {
                case OS_SIG_PULSE_ACK:
                    break;
                case OS_SIG_PWR_ACK:
                    break;
                case OS_SIG_REBOOT:
                    Reboot(task_args_p);
                    break;
                case OS_SIG_SHUTDOWN:
                    Shutdown(task_args_p);
                    break;
                default:
                    OS_LOG_S(D_DEBUG, S_UNDEF_SIG);
                    break;
            }
        } else {
            switch (msg_p->id) {
                default:
                    OS_LOG_S(D_DEBUG, S_UNDEF_MSG);
                    break;
            }
            OS_MessageDelete(msg_p); // free message allocated memory
        }
    }
#if (1 == TIMER_IWDG_ENABLED)
    TIMER_IWDG_Reset();
#endif // TIMER_IWDG_ENABLED
    led_state = (ON == led_state) ? OFF : ON;
    OS_DriverWrite(task_args_p->drv_led_pulse, (void*)&led_state, 1, OS_NULL);

#if (1 == OS_TASK_DEADLOCK_TEST_ENABLED)
    if (OS_TRUE != is_idle) {
        Status s;
        IF_STATUS_OK(s = TaskDeadLockTest()) {
            IF_STATUS_OK(s = TaskDeadLockAction()) {
            } else { OS_LOG_S(D_WARNING, s); }
        } else { OS_LOG_S(D_WARNING, s); }
    } else {
        is_idle = OS_FALSE;
        if (OS_NULL != deadlock_thd) {
            //If the deadlock task released cpu - reset timeout counter.
            OS_TaskTimeoutReset(deadlock_thd);
            deadlock_thd = OS_NULL;
        }
    }
#endif // OS_TASK_DEADLOCK_TEST_ENABLED
}

#if (1 == OS_TASK_DEADLOCK_TEST_ENABLED)
/******************************************************************************/
Status TaskDeadLockTest(void)
{
const U32 task_inf_approx_mem_size = sizeof(OS_TaskStats);
register U32 tasks_count_old = OS_TasksCountGet();
OS_TaskStats* run_stats_old_buf_p = (OS_TaskStats*)OS_Malloc(task_inf_approx_mem_size * tasks_count_old);
OS_TaskStats* run_stats_new_buf_p = OS_NULL;
OS_TaskStats* task_stats_old_p;
OS_TaskStats* task_stats_new_p;
Status s = S_OK;
    if (OS_NULL == run_stats_old_buf_p) {
        OS_LOG_S(D_CRITICAL, (s = S_NO_MEMORY));
        goto error;
    }
    //Get current tasks stats.
    if (tasks_count_old != OS_TasksStatsGet(run_stats_old_buf_p, tasks_count_old, OS_NULL)) {
        s = S_INVALID_VALUE;
        goto error;
    }
    task_stats_old_p = (OS_TaskStats*)&run_stats_old_buf_p[0];
    //Wait for stats saturation.
    OS_TaskDelay(tasks_count_old);
    {
        //Get another one stats measurement.
        register U32 cpu_time_top = 0;
        register U32 tasks_count_new = OS_TasksCountGet();
        run_stats_new_buf_p = (OS_TaskStats*)OS_Malloc(task_inf_approx_mem_size * tasks_count_new);
        if (OS_NULL == run_stats_new_buf_p) {
            OS_LOG_S(D_CRITICAL, (s = S_NO_MEMORY));
            goto error;
        }
        if (tasks_count_new != OS_TasksStatsGet(run_stats_new_buf_p, tasks_count_new, OS_NULL)) {
            s = S_INVALID_VALUE;
            goto error;
        }
        task_stats_new_p = (OS_TaskStats*)&run_stats_new_buf_p[0];
        //Find the most CPU intensive task between measures.
        while (tasks_count_new--) {
            register U32 tasks_count_old_tmp = tasks_count_old;
            TaskHandle_t task_curr_thd = OS_NULL;
            task_stats_old_p = (OS_TaskStats*)&run_stats_old_buf_p[0];
            while (tasks_count_old_tmp--) {
                if (task_stats_new_p->xHandle == task_stats_old_p->xHandle) {
                    task_curr_thd = task_stats_new_p->xHandle;
                    break;
                }
                ++task_stats_old_p;
            }
            if (OS_NULL != task_curr_thd) {
                const U32 cpu_time_delta = task_stats_new_p->ulRunTimeCounter - task_stats_old_p->ulRunTimeCounter;
                if (cpu_time_top < cpu_time_delta) {
                    extern OS_TaskHd OS_TaskHdGet(const TaskHandle_t task_hd);
                    const OS_TaskHd thd = OS_TaskHdGet(task_curr_thd);
                    if (OS_TaskGet() != thd) { //Exclude this SV task!
                        cpu_time_top = cpu_time_delta;
                        deadlock_thd = thd;
                    }
                }
            }
            ++task_stats_new_p;
        }
    }
error:
    OS_Free(run_stats_old_buf_p);
    OS_Free(run_stats_new_buf_p);
    return s;
}

/******************************************************************************/
Status TaskDeadLockAction(void)
{
extern U32  OS_TaskTimeoutGet(const OS_TaskHd thd);
extern void OS_TaskTimeoutDec(const OS_TaskHd thd);
const OS_TaskConfig* task_cfg_p = OS_TaskConfigGet(deadlock_thd);

    if (OS_NULL != task_cfg_p) {
        if (OS_TaskTimeoutGet(deadlock_thd)) {
            OS_TaskTimeoutDec(deadlock_thd);
        } else {
            OS_LOG(D_CRITICAL, "Deadlock detected!");
            OS_ASSERT(S_OK == OS_TaskDelete(deadlock_thd));
            deadlock_thd = OS_NULL;
            if (BIT_TEST(task_cfg_p->attrs, BIT(OS_TASK_ATTR_RECREATE))) {
                OS_ASSERT(S_OK == OS_TaskCreate(task_cfg_p, OS_NULL));
            }
        }
    } else {
        Status s = S_INVALID_REF;
        OS_LOG_S(D_DEBUG, s);
        return s;
    }
    return S_OK;
}
#endif // OS_TASK_DEADLOCK_TEST_ENABLED

/******************************************************************************/
void Reboot(TaskArgs* task_args_p)
{
    OS_LOG(D_INFO, "Reboot system");
    //Prepare system to reboot.
    OS_TaskPower(task_args_p, PWR_SHUTDOWN);
    printf("\nReboot in");
    for (U8 i = 3; i > 0; --i) {
        printf(" ...%d", i);
        OS_TaskDelay(1000);
    }
    OS_SchedulerSuspend();
    OS_CriticalSectionEnter(); // Make sure that no exceptions are to occur until reset.
    HAL_NVIC_SystemReset();
}

/******************************************************************************/
void Shutdown(TaskArgs* task_args_p) {
    OS_LOG(D_INFO, "Shutdown system");
    //Prepare system to shutdown.
    OS_TaskPower(task_args_p, PWR_SHUTDOWN);
    OS_SchedulerSuspend();
    OS_CriticalSectionEnter(); // Make sure that no exceptions are to occur until power off.
    //HAL_PowerOff();
}
