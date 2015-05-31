/***************************************************************************//**
* @file    os_task.c
* @brief   OS Task.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "osal.h"
#include "os_common.h"
#include "os_supervise.h"
#include "os_list.h"
#include "os_mutex.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_mailbox.h"
#include "os_task.h"

//------------------------------------------------------------------------------
#define OS_TASKS_COUNT_MAX          TYPE_VALUE_MAX(OS_TaskId)

//------------------------------------------------------------------------------
typedef struct {
    const OS_TaskConfig* cfg_p;
    OS_QueueHd      stdin_qhd;
    OS_List*        slots_l_p;
    OS_TaskHd       parent;
    OS_TaskArgs     args;
    OS_TaskStats    stats;
    OS_TaskId       id;
    OS_PowerState   power;
    U8              timeout;
} OS_TaskConfigDyn;

//------------------------------------------------------------------------------
/// @brief      Set task power state.
/// @param[in]  thd             Task handle.
/// @param[in]  state           Task power state.
/// @return     #Status.
Status          OS_TaskPowerStateSet(const OS_TaskHd thd, const OS_PowerState state);

/// @brief      Get task slots list.
/// @param[in]  thd             Task handle.
/// @return     Slots list.
const OS_List*  OS_TaskSlotsGet(const OS_TaskHd thd);

/// @brief      Is a single instance task?
/// @param[in]  cfg_p           Task config.
/// @return     #Bool.
static Bool OS_TaskIsSingle(const OS_TaskConfig* cfg_p);

//------------------------------------------------------------------------------
static OS_List os_tasks_list;
static OS_MutexHd os_task_mutex;
static volatile OS_TaskId id_curr;
//static volatile OS_TaskId tasks_count;

/******************************************************************************/
static TaskHandle_t OS_TaskHandleGet(const OS_TaskHd thd);
INLINE TaskHandle_t OS_TaskHandleGet(const OS_TaskHd thd)
{
    if ((OS_TaskHd)(~0UL) == thd) { return OS_NULL; }
    const OS_ListItem* item_l_p = (OS_ListItem*)((OS_THIS_TASK == thd) ? OS_TaskGet() : thd);
    TaskHandle_t task_hd = (TaskHandle_t)OS_ListItemOwnerGet(item_l_p);
    return task_hd;
}

/******************************************************************************/
OS_TaskHd OS_TaskHdGet(const TaskHandle_t task_hd);
OS_TaskHd OS_TaskHdGet(const TaskHandle_t task_hd)
{
OS_ListItem* iter_li_p;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        iter_li_p = OS_ListItemByOwnerGet(&os_tasks_list, task_hd);
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
    return (OS_TaskHd)iter_li_p;
}

/******************************************************************************/
static OS_TaskConfigDyn* OS_TaskConfigDynGet(const OS_TaskHd thd);
INLINE OS_TaskConfigDyn* OS_TaskConfigDynGet(const OS_TaskHd thd)
{
    if ((OS_TaskHd)(~0UL) == thd) { return OS_NULL; }
    const OS_ListItem* item_l_p = (OS_ListItem*)((OS_THIS_TASK == thd) ? OS_TaskGet() : thd);
    OS_TaskConfigDyn* cfg_dyn_p = (OS_TaskConfigDyn*)OS_ListItemValueGet(item_l_p);
    return cfg_dyn_p;
}

/******************************************************************************/
//INLINE
const OS_TaskConfig* OS_TaskConfigGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->cfg_p;
}

/******************************************************************************/
Bool OS_TaskIsSingle(const OS_TaskConfig* cfg_p)
{
OS_TaskHd thd = OS_TaskNextGet(OS_NULL); //get first task in the list.
    while (OS_NULL != thd) {
        OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
        if (OS_NULL == cfg_dyn_p) {
            OS_LOG_S(D_DEBUG, S_INVALID_PTR);
            return OS_FALSE;
        }
        if (!OS_MemCmp(cfg_p, cfg_dyn_p->cfg_p, sizeof(OS_TaskConfig))) {
            return OS_FALSE;
        }
        thd = OS_TaskNextGet(thd);
    }
    return OS_TRUE;
}

/******************************************************************************/
Status OS_TaskInit_(void);
Status OS_TaskInit_(void)
{
Status s = S_OK;
    id_curr = 1;
    //tasks_count = 0;
    os_task_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_task_mutex) { return S_INVALID_PTR; }
    OS_ListInit(&os_tasks_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_tasks_list)) { return S_INVALID_VALUE; }
    return s;
}

/******************************************************************************/
Status OS_TaskCreate(const void* args_p, const OS_TaskConfig* cfg_p, OS_TaskHd* thd_p)
{
Status s = S_UNDEF;
    if (OS_NULL == cfg_p) { return S_INVALID_PTR; }
    if (BIT_TEST(cfg_p->attrs, BIT(OS_TASK_ATTR_SINGLE))) {
        if (OS_TRUE != OS_TaskIsSingle(cfg_p)) {
            OS_LOG(D_DEBUG, "Task is already created!");
            return S_INVALID_TASK;
        }
    }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_OUT_OF_MEMORY; }
    const OS_TaskHd thd = (OS_TaskHd)item_l_p;
    OS_TaskConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_TaskConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_OUT_OF_MEMORY;
    }
    if (cfg_p->storage_size) {
        cfg_dyn_p->args.stor_p = OS_Malloc(cfg_p->storage_size);
        if (OS_NULL == cfg_dyn_p->args.stor_p) {
            OS_ListItemDelete(item_l_p);
            OS_Free(cfg_dyn_p);
            return S_OUT_OF_MEMORY;
        }
        OS_MemSet(cfg_dyn_p->args.stor_p, 0, cfg_p->storage_size);
    } else {
        cfg_dyn_p->args.stor_p = OS_NULL;
    }
    const OS_QueueConfig que_cfg = {
        .len        = (0 == cfg_p->stdin_len) ? 1 : cfg_p->stdin_len, //At least one item queue to create!
        .item_size  = sizeof(OS_Message*)
    };
    const TaskHandle_t task_hd_curr = xTaskGetCurrentTaskHandle();
    if (OS_NULL != task_hd_curr) {
        // TODO(A. Filyanov) Workaround for the sv task in startup sequence.
        IF_STATUS(s = OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) { goto error; }
    }
    TaskHandle_t task_hd;
    // Assign id to the task.
    while (OS_NULL != OS_TaskByIdGet(id_curr)) {
        (0 == ++id_curr) ? ++id_curr : id_curr; // except zero id.
    }
    // Creating StdIo task queues.
    IF_STATUS(s = OS_QueueCreate(&que_cfg, thd, &cfg_dyn_p->stdin_qhd))  { goto error; }
    cfg_dyn_p->cfg_p        = cfg_p;
    cfg_dyn_p->args.args_p  = (OS_NULL != args_p) ? args_p : cfg_p->args_p;
    cfg_dyn_p->id           = id_curr;
    cfg_dyn_p->parent       = OS_TaskGet();
    cfg_dyn_p->slots_l_p    = OS_NULL;
    cfg_dyn_p->timeout      = cfg_dyn_p->cfg_p->timeout;
    OS_CriticalSectionEnter(); { // Atomic section to prevent context switch right after task creation by OS Engine.
        if (pdPASS != xTaskCreate((TaskFunction_t)cfg_p->func_main, cfg_p->name, cfg_p->stack_size,
                                  (void*)&cfg_dyn_p->args, cfg_p->prio_init, &task_hd)) {
            OS_CriticalSectionExit();
            s = S_MODULE;
            goto error;
        }
        vTaskSetApplicationTaskTag(task_hd, (TaskHookFunction_t)thd);
        OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
        OS_ListItemOwnerSet(item_l_p, (OS_Owner)task_hd);
        OS_ListAppend(&os_tasks_list, item_l_p);
    } OS_CriticalSectionExit();
    if (OS_NULL != task_hd_curr) {
        s = OS_MutexRecursiveUnlock(os_task_mutex);
    }
    if (OS_NULL != thd_p) {
        *thd_p = thd;
    }
    //++tasks_count;
    IF_OK(s) {
        if (OS_NULL != cfg_dyn_p->cfg_p->func_power) {
            if (OS_NULL != task_hd_curr) {
                const OS_QueueHd this_task_qhd = OS_TaskStdInGet(OS_THIS_TASK);
                OS_Signal signal = OS_SignalCreate(OS_SIG_PWR, PWR_STARTUP); //Task power "startup" state set.
                IF_OK(s = OS_SignalSend(cfg_dyn_p->stdin_qhd, signal, OS_MSG_PRIO_HIGH)) {
                    OS_Message* msg_p;
                    IF_OK(s = OS_MessageReceive(this_task_qhd, &msg_p, OS_TIMEOUT_POWER)) {
                        if (OS_SignalIs(msg_p)) {
                            if (OS_SIG_PWR_ACK == OS_SignalIdGet(msg_p)) {
                                IF_OK(s = (Status)OS_SignalDataGet(msg_p)) {
                                    signal = OS_SignalCreate(OS_SIG_PWR, PWR_ON); //Task power "on" state set.
                                    IF_OK(s = OS_SignalSend(cfg_dyn_p->stdin_qhd, signal, OS_MSG_PRIO_HIGH)) {
                                        IF_OK(s = OS_MessageReceive(this_task_qhd, &msg_p, OS_TIMEOUT_POWER)) {
                                            if (OS_SignalIs(msg_p)) {
                                                if (OS_SIG_PWR_ACK == OS_SignalIdGet(msg_p)) {
                                                    IF_OK(s = (Status)OS_SignalDataGet(msg_p)) {
                                                        OS_LOG(D_DEBUG, "[TID:%03u]%s: Hello world!", OS_TaskIdGet(thd), cfg_p->name);
                                                    }
                                                } else {
                                                    s = S_INVALID_SIGNAL;
                                                    OS_LOG_S(D_WARNING, s);
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                s = S_INVALID_SIGNAL;
                                OS_LOG_S(D_WARNING, s);
                            }
                        }
                    }
                }
            }
        } else {
            s = OS_TaskPowerStateSet(thd, PWR_UNDEF);
        }
    }
error:
    IF_STATUS(s) {
        if (OS_NULL != task_hd_curr) {
            // TODO(A. Filyanov) Workaround for the sv task in startup sequence.
            IF_OK(s = OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
                //--tasks_count;
                OS_ListItemDelete(item_l_p);
                OS_Free(cfg_dyn_p->args.stor_p);
                OS_Free(cfg_dyn_p);
                if (OS_NULL != task_hd_curr) {
                    s = OS_MutexRecursiveUnlock(os_task_mutex);
                }
            }
        }
    }
    return s;
}

/******************************************************************************/
Status OS_TaskDelete(const OS_TaskHd thd)
{
OS_ListItem* item_l_p = (OS_ListItem*)((OS_THIS_TASK == thd) ? OS_TaskGet() : thd);
OS_TaskConfigDyn* cfg_dyn_p = (OS_TaskConfigDyn*)OS_ListItemValueGet(item_l_p);
Status s = S_OK;

    if (OS_NULL == item_l_p) { return S_INVALID_PTR; }
    IF_OK(s = OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        const OS_TaskConfig* cfg_p = cfg_dyn_p->cfg_p;
        const TaskHandle_t task_hd = (TaskHandle_t)OS_ListItemOwnerGet(item_l_p);
        const OS_TaskId tid = cfg_dyn_p->id;
        IF_STATUS(s = OS_TaskPowerStateSet(thd, PWR_SHUTDOWN)) { goto error; } //TODO(A.Filyanov) Status handler!
        if (OS_NULL != cfg_dyn_p->stdin_qhd) {
            IF_STATUS(s = OS_QueueDelete(cfg_dyn_p->stdin_qhd)) {
                OS_LOG_S(D_WARNING, s);
            }
        }
        if (OS_NULL != cfg_dyn_p->slots_l_p) {
            //Tasks disconnect signal;
            const OS_Signal signal = OS_SignalCreateEx(cfg_dyn_p->id, OS_SIG_TASK_DISCONNECT, 0);
            OS_MessageMulticastSend(cfg_dyn_p->slots_l_p, (OS_Message*)signal, OS_NO_BLOCK, OS_MSG_PRIO_NORMAL);
            OS_ListClear(cfg_dyn_p->slots_l_p);
            OS_Free(cfg_dyn_p->slots_l_p);
        }
        OS_ListItemDelete(item_l_p);
        OS_Free(cfg_dyn_p->args.stor_p);
        OS_Free(cfg_dyn_p);
        //--tasks_count;
error:
        OS_MutexRecursiveUnlock(os_task_mutex);
        OS_LOG(D_DEBUG, "[TID:%03u]%s: Goodbye cruel world!", tid, cfg_p->name);
        vTaskDelete(task_hd); //Task (self-)delete.
    }
    return s;
}

/******************************************************************************/
void OS_TaskDelay(const OS_TimeMs timeout)
{
    vTaskDelay(OS_MS_TO_TICKS(timeout));
}

/******************************************************************************/
void OS_TaskDelayUntil(OS_Tick* tick_last_p, const OS_TimeMs timeout)
{
    vTaskDelayUntil(tick_last_p, OS_MS_TO_TICKS(timeout));
}

/******************************************************************************/
void OS_TaskSuspend(const OS_TaskHd thd)
{
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
    vTaskSuspend(task_hd);
}

/******************************************************************************/
void OS_TaskResume(const OS_TaskHd thd)
{
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
    vTaskResume(task_hd);
}

/******************************************************************************/
OS_TaskHd OS_TaskByHandleGet(const TaskHandle_t task_hd);
INLINE OS_TaskHd OS_TaskByHandleGet(const TaskHandle_t task_hd)
{
    return ((OS_TaskHd)xTaskGetApplicationTaskTag(task_hd));
}

/******************************************************************************/
OS_TaskHd OS_TaskGet(void)
{
const TaskHandle_t task_hd = xTaskGetCurrentTaskHandle();
    if (OS_NULL == task_hd) { return OS_NULL; }
    return OS_TaskByHandleGet(task_hd);
}

/******************************************************************************/
OS_TaskId OS_TaskIdGet(const OS_TaskHd thd)
{
const OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return 0; }
    return cfg_dyn_p->id;
}

/******************************************************************************/
OS_TaskHd OS_TaskByIdGet(const OS_TaskId tid)
{
OS_ListItem* iter_li_p;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_tasks_list));
        while (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
            if (tid == OS_TaskIdGet((OS_TaskHd)iter_li_p)) {
                goto exit;
            }
            iter_li_p = OS_ListItemNextGet(iter_li_p);
        }
        iter_li_p = OS_NULL;
exit:
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
    return (OS_TaskHd)iter_li_p;
}

/******************************************************************************/
ConstStrP OS_TaskNameGet(const OS_TaskHd thd)
{
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
    if (OS_NULL == task_hd) { return ""; }
    return pcTaskGetTaskName(task_hd);
}

/******************************************************************************/
U32 OS_TasksCountGet(void)
{
    return uxTaskGetNumberOfTasks();
}

/******************************************************************************/
U32 OS_TasksStatsGet(OS_TaskStats* stats_p, const U32 stats_count, U32* uptime_p)
{
#if (OS_STATS_ENABLED)
    return (U32)uxTaskGetSystemState(stats_p, stats_count, uptime_p);
#else
    return 0;
#endif // (OS_STATS_ENABLED)
}

/******************************************************************************/
OS_TaskState OS_TaskStateTranslate(const eTaskState e_state);
OS_TaskState OS_TaskStateTranslate(const eTaskState e_state)
{
OS_TaskState state = OS_TASK_STATE_UNDEF;
    switch (e_state) {
        case eReady:
            state = OS_TASK_STATE_READY;
            break;
        case eRunning:
            state = OS_TASK_STATE_RUN;
            break;
        case eBlocked:
            state = OS_TASK_STATE_BLOCK;
            break;
        case eSuspended:
            state = OS_TASK_STATE_SUSPEND;
            break;
        case eDeleted:
            state = OS_TASK_STATE_DELETED;
            break;
        default:
            break;
    }
    return state;
}

/******************************************************************************/
OS_TaskState OS_TaskStateGet(const OS_TaskHd thd)
{
#if (OS_STATS_ENABLED)
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
const eTaskState e_state = eTaskGetState(task_hd);
#else
const eTaskState e_state = eDeleted + 1;
#endif // (OS_STATS_ENABLED)
    return OS_TaskStateTranslate(e_state);
}

/******************************************************************************/
ConstStrP OS_TaskStateNameGet(const OS_TaskState state)
{
static ConstStr undef_str[]     = "undef";
static ConstStr ready_str[]     = "ready";
static ConstStr run_str[]       = "run";
static ConstStr block_str[]     = "block";
static ConstStr suspend_str[]   = "suspend";
static ConstStr deleted_str[]   = "deleted";
ConstStrP state_str             = undef_str;

    switch (state) {
        case OS_TASK_STATE_READY:
            state_str = ready_str;
            break;
        case OS_TASK_STATE_RUN:
            state_str = run_str;
            break;
        case OS_TASK_STATE_BLOCK:
            state_str = block_str;
            break;
        case OS_TASK_STATE_SUSPEND:
            state_str = suspend_str;
            break;
        case OS_TASK_STATE_DELETED:
            state_str = deleted_str;
            break;
        default:
            break;
    }
    return state_str;
}

/******************************************************************************/
OS_TaskHd OS_TaskParentGet(void)
{
    return OS_TaskParentByHdGet(OS_TaskGet());
}

/******************************************************************************/
OS_TaskHd OS_TaskParentByHdGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->parent;
}

/******************************************************************************/
const void* OS_TaskArgumentsGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->args.args_p;
}

/******************************************************************************/
OS_TaskStorage* OS_TaskStorageGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->args.stor_p;
}

/******************************************************************************/
OS_TaskPrio OS_TaskPriorityGet(const OS_TaskHd thd)
{
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
    return (OS_TaskPrio)uxTaskPriorityGet(task_hd);
}

/******************************************************************************/
void OS_TaskPowerPrioritySort(const SortDirection sort_dir);
void OS_TaskPowerPrioritySort(const SortDirection sort_dir)
{
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        //TODO(A.Filyanov) Optimize selective sort function.
        OS_ListItem* item_curr_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_tasks_list));
        OS_ListItem* item_next_p;
        OS_ListItem* item_min_p;
        OS_TaskConfigDyn* cfg_dyn_min_p;
        OS_TaskConfigDyn* cfg_dyn_next_p;

        while (OS_DELAY_MAX != OS_ListItemValueGet(item_curr_p)) {
            item_min_p = item_curr_p;
            item_next_p = OS_ListItemNextGet(item_curr_p);
            while (OS_DELAY_MAX != OS_ListItemValueGet(item_next_p)) {
                cfg_dyn_min_p  = (OS_TaskConfigDyn*)OS_ListItemValueGet(item_min_p);
                cfg_dyn_next_p = (OS_TaskConfigDyn*)OS_ListItemValueGet(item_next_p);

                if (SORT_ASCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg_p->prio_power < cfg_dyn_min_p->cfg_p->prio_power) {
                        item_min_p = item_next_p;
                    }
                } else if (SORT_DESCENDING == sort_dir) {
                    if (cfg_dyn_next_p->cfg_p->prio_power > cfg_dyn_min_p->cfg_p->prio_power) {
                        item_min_p = item_next_p;
                    }
                } else { OS_ASSERT(OS_FALSE); }

                item_next_p = OS_ListItemNextGet(item_next_p);
            }
            if (item_curr_p != item_min_p) {
                OS_ListItemsSwap(item_curr_p, item_min_p);
            }
            item_curr_p = OS_ListItemNextGet(item_min_p);
        }
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
}

/******************************************************************************/
OS_PowerPrio OS_TaskPowerPriorityGet(const OS_TaskHd thd);
OS_PowerPrio OS_TaskPowerPriorityGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
const OS_TaskConfig* cfg_p = cfg_dyn_p->cfg_p;

    if (OS_NULL == cfg_dyn_p) { return OS_PWR_PRIO_UNDEF; }
    if (OS_NULL == cfg_p) { return OS_PWR_PRIO_UNDEF; }
    return cfg_p->prio_power;
}

/******************************************************************************/
OS_PowerState OS_TaskPowerStateGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);

    if (OS_NULL == cfg_dyn_p) { return PWR_UNDEF; }
    return cfg_dyn_p->power;
}

/******************************************************************************/
OS_TaskAttrs OS_TaskAttrsGet(const OS_TaskHd thd);
OS_TaskAttrs OS_TaskAttrsGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);

    if (OS_NULL == cfg_dyn_p) { return U8_MAX; }
    return cfg_dyn_p->cfg_p->attrs;
}

/******************************************************************************/
U32 OS_TaskTimeoutGet(const OS_TaskHd thd);
U32 OS_TaskTimeoutGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);

    if (OS_NULL == cfg_dyn_p) { return U32_MAX; }
    return cfg_dyn_p->timeout;
}

/******************************************************************************/
Status OS_TaskTimeoutDec(const OS_TaskHd thd);
Status OS_TaskTimeoutDec(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);

    if (OS_NULL == cfg_dyn_p) { return S_INVALID_PTR; }
    (OS_TaskConfigDynGet(thd)->timeout)--;
    return S_OK;
}

/******************************************************************************/
Status OS_TaskTimeoutReset(const OS_TaskHd thd);
Status OS_TaskTimeoutReset(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);

    if (OS_NULL == cfg_dyn_p) { return S_INVALID_PTR; }
    cfg_dyn_p->timeout = cfg_dyn_p->cfg_p->timeout;
    return S_OK;
}

/******************************************************************************/
Status OS_TaskPowerStateSet(const OS_TaskHd thd, const OS_PowerState state)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
Status s = S_OK;
    // Do not call func_power() if the state was already being set.
    if (OS_NULL == cfg_dyn_p) { return S_INVALID_PTR; }
    if (state != cfg_dyn_p->power) {
        OS_LOG(D_DEBUG, "Power state: %s", OS_PowerStateNameGet(state));
        if (OS_NULL != cfg_dyn_p->cfg_p->func_power) {
            IF_STATUS(s = cfg_dyn_p->cfg_p->func_power((void*)&cfg_dyn_p->args, state)) {
                return s;
            }
        }
        cfg_dyn_p->power = state;
    }
    return s;
}

/******************************************************************************/
Status OS_TaskPrioritySet(const OS_TaskHd thd, const OS_TaskPrio prio)
{
const TaskHandle_t task_hd = OS_TaskHandleGet(thd);
    vTaskPrioritySet(task_hd, prio);
    return S_OK;
}

/******************************************************************************/
OS_TaskHd OS_TaskByNameGet(ConstStr* name_p)
{
OS_TaskHd thd = OS_NULL;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        OS_ListItem* iter_li_p = (OS_ListItem*)&OS_ListItemLastGet(&os_tasks_list);
        OS_TaskConfigDyn* cfg_dyn_p;

        while (OS_DELAY_MAX != OS_ListItemValueGet(OS_ListItemNextGet(iter_li_p))) {
            iter_li_p = OS_ListItemNextGet(iter_li_p);
            cfg_dyn_p = (OS_TaskConfigDyn*)OS_ListItemValueGet(iter_li_p);
            if (!OS_StrCmp((const char*)name_p, (const char*)cfg_dyn_p->cfg_p->name)) {
                thd = (OS_TaskHd)iter_li_p;
                break;
            }
        }
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
    return thd;
}

/******************************************************************************/
OS_TaskHd OS_TaskNextGet(const OS_TaskHd thd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)thd;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_tasks_list));
            if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_NULL;
            }
        } else {
            if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_ListItemNextGet(iter_li_p);
                if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                    iter_li_p = OS_NULL;
                }
            } else {
                iter_li_p = OS_NULL;
            }
        }
        OS_MutexRecursiveUnlock(os_task_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_TaskHd)iter_li_p;
}

/******************************************************************************/
Status OS_TasksConnect(const OS_TaskHd signal_thd, const OS_TaskHd slot_thd)
{
Status s = S_OK;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        OS_TaskConfigDyn* cfg_dyn_p;
        OS_ListItem* item_l_p;
        const OS_QueueHd slot_qhd = OS_TaskStdInGet(slot_thd);
        if (OS_NULL == slot_qhd) { s = S_INVALID_PTR; goto error; }
        cfg_dyn_p = OS_TaskConfigDynGet(signal_thd);
        if (OS_NULL == cfg_dyn_p) { s = S_INVALID_PTR; goto error; }
        if (OS_NULL == cfg_dyn_p->slots_l_p) {
            cfg_dyn_p->slots_l_p = OS_Malloc(sizeof(OS_List));
            if (OS_NULL == cfg_dyn_p->slots_l_p) { s = S_OUT_OF_MEMORY; goto error; }
            OS_ListInit(cfg_dyn_p->slots_l_p);
            if (OS_TRUE != OS_ListIsInitialised(cfg_dyn_p->slots_l_p)) {
                OS_Free(cfg_dyn_p->slots_l_p);
                cfg_dyn_p->slots_l_p = OS_NULL;
                s = S_INVALID_VALUE;
                goto error;
            }
        }
        item_l_p = OS_ListItemCreate();
        if (OS_NULL == item_l_p) {
            OS_Free(cfg_dyn_p->slots_l_p);
            cfg_dyn_p->slots_l_p = OS_NULL;
            s = S_OUT_OF_MEMORY;
            goto error;
        }
        OS_ListItemValueSet(item_l_p, (OS_Value)slot_qhd);
        OS_ListItemOwnerSet(item_l_p, (OS_Owner)slot_thd);
        OS_ListAppend(cfg_dyn_p->slots_l_p, item_l_p);
error:
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
    IF_OK(s) {
        OS_LOG(D_DEBUG, "Tasks are connected:\r\nsignal [%03u] %s > slot [%03u] %s",
               OS_TaskIdGet(signal_thd),
               OS_TaskNameGet(signal_thd),
               OS_TaskIdGet(slot_thd),
               OS_TaskNameGet(slot_thd));
    }
    return s;
}

/******************************************************************************/
Status OS_TasksDisconnect(const OS_TaskHd signal_thd, const OS_TaskHd slot_thd)
{
Status s = S_OK;
    IF_OK(OS_MutexRecursiveLock(os_task_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        OS_ListItem* item_l_p;
        OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(signal_thd);
        if (OS_NULL == cfg_dyn_p) { s = S_INVALID_PTR; goto error; }
        item_l_p = OS_ListItemByOwnerGet(cfg_dyn_p->slots_l_p, slot_thd);
        if (OS_NULL == item_l_p) { s = S_INVALID_VALUE; goto error; }
        OS_ListItemDelete(item_l_p);
        if (OS_TRUE == OS_ListIsEmpty(cfg_dyn_p->slots_l_p)) {
            OS_Free(cfg_dyn_p->slots_l_p);
            cfg_dyn_p->slots_l_p = OS_NULL;
        }
error:
        OS_MutexRecursiveUnlock(os_task_mutex);
    }
    IF_OK(s) {
        OS_LOG(D_DEBUG, "Tasks are disconnected:\r\nsignal [%03u] %s | slot [%03u] %s",
               OS_TaskIdGet(signal_thd),
               OS_TaskNameGet(signal_thd),
               OS_TaskIdGet(slot_thd),
               OS_TaskNameGet(slot_thd));
    }
    return s;
}

/******************************************************************************/
const OS_List* OS_TaskSlotsGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->slots_l_p;
}

/******************************************************************************/
OS_QueueHd OS_TaskStdInGet(const OS_TaskHd thd)
{
OS_TaskConfigDyn* cfg_dyn_p = OS_TaskConfigDynGet(thd);
    if (OS_NULL == cfg_dyn_p) { return OS_NULL; }
    return cfg_dyn_p->stdin_qhd;
}

/******************************************************************************/
OS_QueueHd OS_TaskSvStdInGet(void)
{
extern volatile OS_QueueHd sv_stdin_qhd;
    return sv_stdin_qhd;
}