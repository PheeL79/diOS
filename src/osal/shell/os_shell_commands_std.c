/**************************************************************************//**
* @file    os_shell_commands_std.c
* @brief   OS shell standart commands.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include "hal.h"
#include "osal.h"
#include "os_debug.h"
#include "os_common.h"
#include "os_memory.h"
#include "os_task.h"
#include "os_debug.h"
#include "os_signal.h"
#include "os_timer.h"
#include "os_trigger.h"
#include "os_driver.h"
#include "os_mailbox.h"
#include "os_environment.h"
#include "os_shell_commands_std.h"
#include "os_shell.h"

//------------------------------------------------------------------------------
/// @brief Commands descriptions following are:
/// @brief Command descritpion, help string, command handler function

//------------------------------------------------------------------------------
#ifdef OS_SHELL_HELP_ENABLED
static ConstStr cmd_help[] = "help";
/******************************************************************************/
static Status OS_ShellCmdHelpHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdHelpHandler(const U32 argc, ConstStrP argv[])
{
OS_ShellCommandHd cmd_hd = OS_ShellCommandNextGet(OS_NULL);
    //Ignore 'help' command(first one).
    cmd_hd = OS_ShellCommandNextGet(cmd_hd);
    while (cmd_hd) {
        printf("\n%-21s :%s", cmd_hd->command, cmd_hd->help_brief);
        cmd_hd = OS_ShellCommandNextGet(cmd_hd);
    }
    return S_OK;
}
#endif // OS_SHELL_HELP_ENABLED

//------------------------------------------------------------------------------
#define OS_CMD_CLEAR_STRING             "clear"
static ConstStr cmd_clear[]             = OS_CMD_CLEAR_STRING;
static ConstStr cmd_help_brief_clear[]  = "Clear screen.";

static ConstStr cmd_cls[]           = "cls";
static ConstStr cmd_help_brief_cls[]= "Acronym of \'" OS_CMD_CLEAR_STRING "\' command.";
/******************************************************************************/
static Status OS_ShellCmdClearHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdClearHandler(const U32 argc, ConstStrP argv[])
{
    return OS_ShellCls();
}

//------------------------------------------------------------------------------
static ConstStr cmd_echo[]              = "echo";
static ConstStr cmd_help_brief_echo[]   = "Display the message.";
/******************************************************************************/
static Status OS_ShellCmdEchoHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdEchoHandler(const U32 argc, ConstStrP argv[])
{
register U32 argc_count = 0;
    while (argc > argc_count) {
        printf("\n%s", argv[argc_count]);
        ++argc_count;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_d[]             = "d";
static ConstStr cmd_help_brief_d[]  = "Memory dump.";
/******************************************************************************/
static Status OS_ShellCmdDHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdDHandler(const U32 argc, ConstStrP argv[])
{
#define IS_PRINT(c)     ((OS_ASCII_TILDE >= c) && (OS_ASCII_SPACE <= c))
const U8 step = 16;
    //Convert address to number.
    const U8* addr_str = (const U8*)argv[0];
    U8 base = ('x' == *(U8*)(addr_str + 1)) ? 16 : 10;
    Long addr = OS_StrToL((const char*)addr_str, OS_NULL, base);
    if (0 > addr) { return S_INVALID_PTR; }
//    OS_MemoryPool mem_pool = OS_MEM_UNDEF;
//    OS_MemoryStat mem_stat;
    //WARNING!!! Allow all of memory regions! Please be careful with the address value!
    Bool is_valid = OS_TRUE;//OS_FALSE;
    //Check address.
//    while (OS_MEM_UNDEF != (mem_pool = OS_MemoryPoolNextGet(mem_pool))) {
//        IF_STATUS(OS_MemoryStatGet(mem_pool, &mem_stat)) { return S_INVALID_VALUE; }
//        if ((addr >= mem_stat.desc.addr) && (addr <= (mem_stat.desc.addr + mem_stat.desc.size))) {
//            is_valid = OS_TRUE;
//            break;
//        }
//    }
    if (OS_TRUE != is_valid) { return S_INVALID_VALUE; }
    const U8* size_str = (const U8*)argv[1];
    base = ('x' == *(U8*)(size_str + 1)) ? 16 : 10;
    Long size = OS_StrToL((const char*)size_str, OS_NULL, base);
    if (0 == size) { return S_OK; }
    while (0 < size) {
        U8* p;
        printf("\n0x%-9X ", addr);
        p = (U8*)addr;
        for (register U8 i = 0; i < (step / 2); ++i) {
            printf("%02X ", *p++);
        }
        putchar(' ');
        for (register U8 i = 0; i < (step / 2); ++i) {
            printf("%02X ", *p++);
        }
        putchar(' ');
        p = (U8*)addr;
        for (register U8 i = 0; i < (step / 2); ++i) {
            U8 c = IS_PRINT(*p) ? *p : '.';
            printf("%c", c);
            ++p;
        }
        putchar(' ');
        for (register U8 i = 0; i < (step / 2); ++i) {
            U8 c = IS_PRINT(*p) ? *p : '.';
            printf("%c", c);
            ++p;
        }
        addr += step;
        size -= step;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_printenv[]              = "printenv";
static ConstStr cmd_help_brief_printenv[]   = "List all the environment variables.";
/******************************************************************************/
static Status OS_ShellCmdPrintEnvHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdPrintEnvHandler(const U32 argc, ConstStrP argv[])
{
ConstStrP var_name_p = OS_NULL;
    while (OS_NULL != (var_name_p = OS_EnvVariableNextGet(var_name_p))) {
        printf("\n%-21s :%s", var_name_p, OS_EnvVariableGet(var_name_p));
    }
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_setenv[]            = "setenv";
static ConstStr cmd_help_brief_setenv[] = "Set the environment variable.";
/******************************************************************************/
static Status OS_ShellCmdSetEnvHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdSetEnvHandler(const U32 argc, ConstStrP argv[])
{
U32 value_idx = 1;
    if ((3 == argc) && ('=' != *argv[1])) { return S_INVALID_VALUE; }
    if (3 == argc) {
        value_idx = 2;
    }
    return OS_EnvVariableSet(argv[0], argv[value_idx], OS_NULL);
}

//------------------------------------------------------------------------------
static ConstStr cmd_unsetenv[]              = "unsetenv";
static ConstStr cmd_help_brief_unsetenv[]   = "Unset the environment variable.";
/******************************************************************************/
static Status OS_ShellCmdUnsetEnvHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdUnsetEnvHandler(const U32 argc, ConstStrP argv[])
{
    return OS_EnvVariableDelete(argv[0]);
}

//------------------------------------------------------------------------------
static ConstStr cmd_st[]            = "st";
static ConstStr cmd_help_brief_st[] = "System statistics.";
/******************************************************************************/
static void OS_ShellCmdStHandlerMemHelper(void);
void OS_ShellCmdStHandlerMemHelper(void)
{
OS_MemoryPool  mem_pool = OS_MEM_UNDEF;
OS_MemoryStats mem_stats;

    printf("\n%-16s %-12s %-12s %-6s %-12s %-12s",
           "Name", "Address", "Size", "Block", "Used", "Free");
    while (OS_MEM_UNDEF != (mem_pool = OS_MemoryPoolNextGet(mem_pool))) {
        IF_STATUS(OS_MemoryStatsGet(mem_pool, &mem_stats)) { return; }
        printf("\n%-16s 0x%-10X %-12d %-6d %-12d %-12d",
               mem_stats.desc.name_p,
               mem_stats.desc.addr,
               mem_stats.desc.size,
               mem_stats.desc.block_size,
               mem_stats.used,
               mem_stats.free);
    }
}

/******************************************************************************/
static void OS_ShellCmdStHandlerTskHelper(void);
void OS_ShellCmdStHandlerTskHelper(void)
{
const U32 task_inf_approx_mem_size = sizeof(OS_TaskStats);
register U32 tasks_count = OS_TasksCountGet();
OS_TaskStats* run_stats_buf_p = (OS_TaskStats*)OS_Malloc(task_inf_approx_mem_size * tasks_count);
OS_TaskStats* task_stats_p;
U32 uptime;

    if (OS_NULL == run_stats_buf_p) { return; }
    printf("\n%-12s %-3s %-4s %-3s %-4s %-7s %-10s %-4s %-5s %-5s %-4s %-5s",
           "Name", "TId", "PTId", "Pri", "PriP", "Power", "State", "CPU", "Store", "Stack", "Free", "StdIn");
    if (tasks_count != OS_TasksStatsGet(run_stats_buf_p, tasks_count, &uptime)) { goto error; }
    uptime /= 100UL; //For percentage calculations.
    if (0 >= uptime) { goto error; } //Avoid divide by zero errors.
    task_stats_p = (OS_TaskStats*)&run_stats_buf_p[0];
    while (tasks_count--) {
        extern OS_TaskHd OS_TaskByHandleGet(const TaskHandle_t task_hd);
        extern OS_TaskState OS_TaskStateTranslate(const eTaskState e_state);

        const OS_TaskHd thd = OS_TaskByHandleGet(task_stats_p->xHandle);
        const OS_TaskHd par_thd = OS_TaskParentByHdGet(thd);
        const OS_TaskConfig* cfg_p = OS_TaskConfigGet(thd);
        const OS_TaskState task_state = OS_TaskStateTranslate(task_stats_p->eCurrentState);

        OS_TaskId tid = OS_TaskIdGet(thd);
        OS_TaskId par_id = (OS_NULL == par_thd) ? 0 : OS_TaskIdGet(par_thd);
        OS_PowerPrio power_prio = cfg_p->prio_power;
        OS_PowerState power_state = OS_TaskPowerStateGet(thd);
        U16 store_size = cfg_p->storage_size;
        U16 stack_size = cfg_p->stack_size;
        U8 stdin_len = cfg_p->stdin_len;
        if (OS_NULL == thd) { // OS Engine tasks.
            tid         = 0;
            par_id      = 0;
            power_prio  = 0;
            power_state = 0;
            store_size  = 0;
            stack_size  = 0;
            stdin_len   = 0;
        }
        StrP cpu_str_buf[5];
        //TODO(A. Filyanov) Check for timer counter overflow! Otherwise -> wrong cpu usage value!
        snprintf((char*)&cpu_str_buf, sizeof(cpu_str_buf), "%d%%", (U32)(task_stats_p->ulRunTimeCounter / uptime));
        printf("\n%-12s %-3d %-4d %-3d %-4d %-7s %-10s %-4s %-5d %-5d %-4d %-3d",
               task_stats_p->pcTaskName,
               tid,
               par_id,
               task_stats_p->uxCurrentPriority,
               power_prio,
               OS_PowerStateNameGet(power_state),
               OS_TaskStateNameGet(task_state),
               (const char*)cpu_str_buf,
               store_size,
               stack_size,
               task_stats_p->usStackHighWaterMark,
               stdin_len);
        ++task_stats_p;
    }
error:
    OS_Free(run_stats_buf_p);
}

/******************************************************************************/
static void OS_ShellCmdStHandlerQueHelper(void);
void OS_ShellCmdStHandlerQueHelper(void)
{
OS_TaskHd thd;
OS_QueueHd qhd = OS_NULL;

    printf("\n%-12s %-4s %-4s %-6s %-6s %-12s %-12s",
           "Parent", "PTId", "Len", "ISize", "Items", "Sended", "Received");
    while (OS_NULL != (qhd = OS_QueueNextGet(qhd))) {
        OS_QueueConfig que_config;
        OS_QueueStats que_stats;
        IF_STATUS(OS_QueueConfigGet(qhd, &que_config)) { return; }
        IF_STATUS(OS_QueueStatsGet(qhd, &que_stats)) { return; }
        if (OS_NULL == (thd = OS_QueueParentGet(qhd))) {
            printf("\nTask undef!");
        } else {
            printf("\n%-12s %-4d %-4d %-6d %-6d %-12d %-12d",
                   OS_TaskNameGet(thd),
                   OS_TaskIdGet(thd),
                   que_config.len,
                   que_config.item_size,
                   OS_QueueItemsCountGet(qhd),
                   que_stats.sended,
                   que_stats.received);
        }
    }
}

/******************************************************************************/
static void OS_ShellCmdStHandlerDrvHelper(void);
void OS_ShellCmdStHandlerDrvHelper(void)
{
OS_DriverHd dhd = OS_NULL;

    printf("\n%-8s %-6s %-7s %-4s %-6s %-12s %-12s %-8s %-9s",
           "Name", "State", "Power", "PriP", "Owners", "Sended", "Received", "Errors", "Status");
    while (OS_NULL != (dhd = OS_DriverNextGet(dhd))) {
        OS_DriverStats drv_stats;
        IF_STATUS(OS_DriverStatsGet(dhd, &drv_stats)) { return; }
        const OS_DriverConfig* drv_cfg_p = OS_DriverConfigGet(dhd);
        if (OS_NULL == drv_cfg_p) { return; }
        printf("\n%-8s %-6s %-7s %-4d %-6d %-12d %-12d %-8d %-9s",
               drv_cfg_p->name,
               OS_DriverStateNameGet(drv_stats.state),
               OS_PowerStateNameGet(drv_stats.power),
               drv_cfg_p->prio_power,
               drv_stats.owners,
               drv_stats.sended,
               drv_stats.received,
               drv_stats.errors_cnt,
               StatusStringGet(drv_stats.status_last, STATUS_ITEMS_COMMON));
    }
}

/******************************************************************************/
#if (OS_TIMERS_ENABLED)
static void OS_ShellCmdStHandlerTimHelper(void);
void OS_ShellCmdStHandlerTimHelper(void)
{
OS_TimerHd timer_hd = OS_NULL;

    printf("\n%-8s %-5s %-3s %-8s %-10s %-12s %-4s",
           "Name", "TimId", "Act", "Options", "Period", "Slot", "STId");
    while (OS_NULL != (timer_hd = OS_TimerNextGet(timer_hd))) {
        OS_TimerStats tim_stats;
        IF_STATUS(OS_TimerStatsGet(timer_hd, &tim_stats)) { return; }
        printf("\n%-8s %-5d %-3s %-8d %-10d %-12s %-4d",
               tim_stats.name_p,
               tim_stats.id,
               (OS_TRUE == OS_TimerIsActive(timer_hd) ? "on" : "off"),
               tim_stats.options,
               tim_stats.period,
               OS_TaskNameGet(tim_stats.slot),
               OS_TaskIdGet(tim_stats.slot));
    }
}
#endif //(OS_TIMERS_ENABLED)

/******************************************************************************/
#if (OS_TRIGGERS_ENABLED)
static void OS_ShellCmdStHandlerTriHelper(void);
void OS_ShellCmdStHandlerTriHelper(void)
{
OS_TriggerHd tigger_hd = OS_NULL;

    printf("\n%-8s %-5s %-3s %-8s %-10s %-12s %-4s %-5s %-4s",
           "Name", "TimId", "Act", "Options", "Period", "Slot", "STId", "State", "Item");
    while (OS_NULL != (tigger_hd = OS_TriggerNextGet(tigger_hd))) {
        OS_TriggerStats trigger_stats;
        IF_STATUS(OS_TriggerStatsGet(tigger_hd, &trigger_stats)) { return; }
        const OS_TimerStats* timer_stats_p = &trigger_stats.timer_stats;
        const OS_TimerHd timer_hd = OS_TimerByIdGet(timer_stats_p->id);
        if (OS_NULL == timer_hd) { return; }
        printf("\n%-8s %-5d %-3s %-8d %-10d %-12s %-4d %-5d 0x%-8x",
               timer_stats_p->name_p,
               timer_stats_p->id,
               (OS_TRUE == OS_TimerIsActive(timer_hd) ? "on" : "off"),
               timer_stats_p->options,
               timer_stats_p->period,
               OS_TaskNameGet(timer_stats_p->slot),
               OS_TaskIdGet(timer_stats_p->slot),
               trigger_stats.state,
               trigger_stats.item_p);
    }
}
#endif //(OS_TRIGGERS_ENABLED)

/******************************************************************************/
#if (OS_NETWORK_ENABLED)
static void OS_ShellCmdStHandlerNetHelper(void);
void OS_ShellCmdStHandlerNetHelper(void)
{
}
#endif //(OS_NETWORK_ENABLED)

/******************************************************************************/
static Status OS_ShellCmdStHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdStHandler(const U32 argc, ConstStrP argv[])
{
typedef struct {
    ConstStrP cmd;
    void (*handler)(void);
} CommandHandler;
CommandHandler cmd_handlers_v[] = {
    { "mem", OS_ShellCmdStHandlerMemHelper }, //memory
    { "tsk", OS_ShellCmdStHandlerTskHelper }, //tasks
    { "que", OS_ShellCmdStHandlerQueHelper }, //queues
    { "drv", OS_ShellCmdStHandlerDrvHelper }, //drivers
#if (OS_TIMERS_ENABLED)
    { "tim", OS_ShellCmdStHandlerTimHelper }, //timers
#endif //(OS_TIMERS_ENABLED)
#if (OS_TRIGGERS_ENABLED)
    { "tri", OS_ShellCmdStHandlerTriHelper }, //triggers
#endif //(OS_TRIGGERS_ENABLED)
#if (OS_FILE_SYSTEM_ENABLED)
//    { "fs",  OS_ShellCmdStHandlerFsHelper  }, //file system
#endif //(OS_FILE_SYSTEM_ENABLED)
#if (OS_NETWORK_ENABLED)
    { "net", OS_ShellCmdStHandlerNetHelper }, //network itf
#endif //(OS_NETWORK_ENABLED)
    { OS_NULL }
};
    for (Size i = 0; OS_NULL != cmd_handlers_v[i].cmd; ++i) {
        if (!OS_StrCmp(cmd_handlers_v[i].cmd, (char const*)argv[0])) {
            cmd_handlers_v[i].handler();
            return S_OK;
        }
    }
    return S_INVALID_VALUE;
}

//------------------------------------------------------------------------------
static ConstStr cmd_kill[]              = "kill";
static ConstStr cmd_help_brief_kill[]   = "Kill the task.";
/******************************************************************************/
static Status OS_ShellCmdKillHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdKillHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_OK;
    //Convert TaskId to number.
    const U8* tid_str = (const U8*)argv[0];
    U8 base = ('x' == *(U8*)(tid_str + 1)) ? 16 : 10;
    Long tid = OS_StrToL((const char*)tid_str, OS_NULL, base);
    if (0 > tid) { return S_INVALID_PTR; }

    const OS_Signal signal = OS_SignalCreate(OS_SIG_PWR, PWR_SHUTDOWN);
    const OS_TaskHd dst_thd = OS_TaskByIdGet(tid);
    if (OS_NULL != dst_thd) {
        const OS_QueueHd dst_stdin_qhd = OS_TaskStdInGet(dst_thd);
        if (OS_NULL != dst_stdin_qhd) {
            OS_SignalSend(dst_stdin_qhd, signal, OS_MSG_PRIO_NORMAL);
            const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
            OS_Message* msg_p;
            do {
                IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
                    OS_LOG_S(L_WARNING, S_INVALID_MESSAGE);
                    return S_INVALID_MESSAGE;
                } else {
                    if (OS_SignalIs(msg_p)) {
                        switch (OS_SignalIdGet(msg_p)) {
                            case OS_SIG_PWR_ACK:
                                IF_STATUS(s = (Status)OS_SignalDataGet(msg_p)) {
                                }
                                s = OS_TaskDelete(dst_thd);
                                return s;
                                break;
                            case OS_SIG_STDOUT: //emited by OS_Log
                                break;
                            default:
                                OS_LOG_S(L_DEBUG_1, S_INVALID_SIGNAL);
                                break;
                        }
                    } else {
                        switch (msg_p->id) {
                            default:
                                OS_LOG_S(L_DEBUG_1, S_INVALID_MESSAGE);
                                break;
                        }
                        OS_MessageDelete(msg_p); // free message allocated memory
                    }
                }
            } while (OS_TRUE);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_time[]              = "time";
static ConstStr cmd_help_brief_time[]   = "Display current time.";
/******************************************************************************/
static Status OS_ShellCmdTimeHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdTimeHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_OK;
    if (!OS_StrCmp("set", (char const*)argv[0])) {
        if ((2 < argc) || (1 > argc)) { return S_INVALID_ARGS_COUNT; }
        OS_DateTime time = OS_TimeStringParse(argv[1]);
        s = OS_TimeSet(OS_TIME_UNDEF, &time);
    } else {
        const Locale locale = OS_LocaleGet();
        OS_DateTime time;
        s = OS_TimeGet(OS_TIME_LOCAL, &time);
        if (LOC_RU == locale) {
            printf("\n%02d" OS_LOCALE_TIME_DELIM_RU "%02d" OS_LOCALE_TIME_DELIM_RU "%02d",
                   time.hours, time.minutes, time.seconds);
        } else {
            printf("\n%02d" OS_LOCALE_TIME_DELIM_EN "%02d" OS_LOCALE_TIME_DELIM_EN "%02d",
                   time.hours, time.minutes, time.seconds);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_date[]              = "date";
static ConstStr cmd_help_brief_date[]   = "Display current date.";
/******************************************************************************/
static Status OS_ShellCmdDateHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdDateHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_OK;
    if (!OS_StrCmp("set", (char const*)argv[0])) {
        if ((2 < argc) || (1 > argc)) { return S_INVALID_ARGS_COUNT; }
        OS_DateTime date = OS_DateStringParse(argv[1]);
        s = OS_DateSet(OS_DATE_UNDEF, &date);
    } else {
        const Locale locale = OS_LocaleGet();
        OS_DateTime date;
        s = OS_DateGet(OS_DATE_UNDEF, &date);
        if (LOC_RU == locale) {
            printf("\n%-10s %02d" OS_LOCALE_DATE_DELIM_RU "%02d" OS_LOCALE_DATE_DELIM_RU "%04d",
                   OS_TimeNameDayOfWeekGet((OS_TimeWeekDay)date.weekday, locale),
                   date.day, date.month, date.year);
        } else {
            printf("\n%-10s %02d" OS_LOCALE_DATE_DELIM_EN "%02d" OS_LOCALE_DATE_DELIM_EN "%04d",
                   OS_TimeNameDayOfWeekGet((OS_TimeWeekDay)date.weekday, locale),
                   date.month, date.day, date.year);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_reboot[]            = "reboot";
static ConstStr cmd_help_brief_reboot[] = "Reboot the device.";
/******************************************************************************/
static Status OS_ShellCmdRebootHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdRebootHandler(const U32 argc, ConstStrP argv[])
{
const OS_Signal signal = OS_SignalCreate(OS_SIG_REBOOT, 0);
const OS_QueueHd sv_stdin_qhd = OS_TaskStdInGet(OS_TaskParentGet());

    OS_SignalSend(OS_TaskSvStdInGet(), signal, OS_MSG_PRIO_HIGH);
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_shutdown[]              = "shutdown";
static ConstStr cmd_help_brief_shutdown[]   = "Shutdown the device.";
/******************************************************************************/
static Status OS_ShellCmdShutdownHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdShutdownHandler(const U32 argc, ConstStrP argv[])
{
const OS_Signal signal = OS_SignalCreate(OS_SIG_SHUTDOWN, 0);
const OS_QueueHd sv_stdin_qhd = OS_TaskStdInGet(OS_TaskParentGet());

    OS_SignalSend(OS_TaskSvStdInGet(), signal, OS_MSG_PRIO_HIGH);
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr empty_str[] = "";
static const OS_ShellCommandConfig cmd_cfg_std[] = {
    { cmd_help,     empty_str,                  empty_str,        OS_ShellCmdHelpHandler,     0,    0,      OS_SHELL_OPT_UNDEF  },
    { cmd_echo,     cmd_help_brief_echo,        empty_str,        OS_ShellCmdEchoHandler,     1,    255,    OS_SHELL_OPT_UNDEF  },
    { cmd_d,        cmd_help_brief_d,           empty_str,        OS_ShellCmdDHandler,        2,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_clear,    cmd_help_brief_clear,       empty_str,        OS_ShellCmdClearHandler,    0,    0,      OS_SHELL_OPT_UNDEF  },
    { cmd_cls,      cmd_help_brief_cls,         empty_str,        OS_ShellCmdClearHandler,    0,    0,      OS_SHELL_OPT_UNDEF  },
    { cmd_printenv, cmd_help_brief_printenv,    empty_str,        OS_ShellCmdPrintEnvHandler, 0,    0,      OS_SHELL_OPT_UNDEF  },
    { cmd_setenv,   cmd_help_brief_setenv,      empty_str,        OS_ShellCmdSetEnvHandler,   2,    3,      OS_SHELL_OPT_UNDEF  },
    { cmd_unsetenv, cmd_help_brief_unsetenv,    empty_str,        OS_ShellCmdUnsetEnvHandler, 1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_st,       cmd_help_brief_st,          empty_str,        OS_ShellCmdStHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_kill,     cmd_help_brief_kill,        empty_str,        OS_ShellCmdKillHandler,     1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_time,     cmd_help_brief_time,        empty_str,        OS_ShellCmdTimeHandler,     0,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_date,     cmd_help_brief_date,        empty_str,        OS_ShellCmdDateHandler,     0,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_reboot,   cmd_help_brief_reboot,      empty_str,        OS_ShellCmdRebootHandler,   0,    0,      OS_SHELL_OPT_UNDEF  },
    { cmd_shutdown, cmd_help_brief_shutdown,    empty_str,        OS_ShellCmdShutdownHandler, 0,    0,      OS_SHELL_OPT_UNDEF  }
};

/******************************************************************************/
Status OS_ShellCommandsStdInit(void)
{
    //Create and register standart shell commands.
    for (Size i = 0; i < ITEMS_COUNT_GET(cmd_cfg_std, OS_ShellCommandConfig); ++i) {
        const OS_ShellCommandConfig* cmd_cfg_p = &cmd_cfg_std[i];
        if (OS_NULL != cmd_cfg_p->command) {
            IF_STATUS(OS_ShellCommandCreate(cmd_cfg_p)) {
                OS_ASSERT(OS_FALSE);
            }
        }
    }
    return S_OK;
}
