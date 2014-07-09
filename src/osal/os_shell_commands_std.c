/**************************************************************************//**
* @file    os_shell_commands_std.c
* @brief   OS shell standart commands.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "osal.h"
#include "os_debug.h"
#include "os_common.h"
#include "os_memory.h"
#include "os_task.h"
#include "os_debug.h"
#include "os_signal.h"
#include "os_timer.h"
#include "os_event.h"
#include "os_driver.h"
#include "os_message.h"
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
static Status OS_ShellCmdHelpHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdHelpHandler(const U32 argc, ConstStrPtr argv[])
{
OS_ShellCommandHd cmd_hd = OS_ShellCommandNextGet(SHELL_COMMAND_UNDEF);
    //Ignore 'help' command(first one).
    cmd_hd = OS_ShellCommandNextGet(cmd_hd);
    while (SHELL_COMMAND_UNDEF != cmd_hd) {
        printf("\n%-21s :%s", cmd_hd->command, cmd_hd->help_brief);
        cmd_hd = OS_ShellCommandNextGet(cmd_hd);
    }
    return S_OK;
}
#endif // OS_SHELL_HELP_ENABLED

//------------------------------------------------------------------------------
#define OS_CMD_CLEAR_STRING         "clear"
static ConstStr cmd_clear[]             = OS_CMD_CLEAR_STRING;
static ConstStr cmd_help_brief_clear[]  = "Clear screen.";

static ConstStr cmd_cls[]           = "cls";
static ConstStr cmd_help_brief_cls[]= "Acronym of \'" OS_CMD_CLEAR_STRING "\' command.";
/******************************************************************************/
static Status OS_ShellCmdClearHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdClearHandler(const U32 argc, ConstStrPtr argv[])
{
    return OS_ShellCls();
}

//------------------------------------------------------------------------------
static ConstStr cmd_echo[]              = "echo";
static ConstStr cmd_help_brief_echo[]   = "Display the message.";
/******************************************************************************/
static Status OS_ShellCmdEchoHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdEchoHandler(const U32 argc, ConstStrPtr argv[])
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
static Status OS_ShellCmdDHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdDHandler(const U32 argc, ConstStrPtr argv[])
{
#define IS_PRINT(c)     ((OS_ASCII_TILDE >= c) && (OS_ASCII_SPACE <= c))
const U8 step = 16;
    //Convert address to number.
    const U8* addr_str = (const U8*)argv[0];
    U8 base = ('x' == *(U8*)(addr_str + 1)) ? 16 : 10;
    LNG addr = strtol((const char*)addr_str, OS_NULL, base);
    if (0 > addr) { return S_INVALID_REF; }
//    OS_MemoryType mem_type = OS_MEM_UNDEF;
//    OS_MemoryStat mem_stat;
    //WARNING!!! Allow all of memory regions! Please be careful with the address value!
    BL is_valid = OS_TRUE;//OS_FALSE;
    //Check address.
//    while (OS_MEM_UNDEF != (mem_type = OS_MemoryTypeHeapNextGet(mem_type))) {
//        IF_STATUS(OS_MemoryStatGet(mem_type, &mem_stat)) { return S_INVALID_VALUE; }
//        if ((addr >= mem_stat.desc.addr) && (addr <= (mem_stat.desc.addr + mem_stat.desc.size))) {
//            is_valid = OS_TRUE;
//            break;
//        }
//    }
    if (OS_TRUE != is_valid) { return S_INVALID_VALUE; }
    const U8* size_str = (const U8*)argv[1];
    base = ('x' == *(U8*)(size_str + 1)) ? 16 : 10;
    LNG size = strtol((const char*)size_str, OS_NULL, base);
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
static Status OS_ShellCmdPrintEnvHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdPrintEnvHandler(const U32 argc, ConstStrPtr argv[])
{
ConstStrPtr var_name_p = OS_NULL;
    while (OS_NULL != (var_name_p = OS_EnvVariableNextGet(var_name_p))) {
        printf("\n%-21s :%s", var_name_p, OS_EnvVariableGet(var_name_p));
    }
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_setenv[]            = "setenv";
static ConstStr cmd_help_brief_setenv[] = "Set the environment variable.";
/******************************************************************************/
static Status OS_ShellCmdSetEnvHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdSetEnvHandler(const U32 argc, ConstStrPtr argv[])
{
U32 value_idx = 1;
    if ((3 == argc) && ('=' != *argv[1])) { return S_INVALID_VALUE; }
    if (3 == argc) {
        value_idx = 2;
    }
    // Exclusive environment variables.
    Status s = S_OK;
    if (!strcmp("locale", (char const*)argv[0])) {
        s = OS_LocaleSet(argv[value_idx]);
    } else if (!strcmp("stdio", (char const*)argv[0])) {
        s = OS_StdIoSet(argv[value_idx]);
    } else if (!strcmp("log_level", (char const*)argv[0])) {
        s = OS_LogLevelSet(argv[value_idx]);
    } else if (!strcmp(OS_ENV_POWER_STR, (char const*)argv[0])) {
        s = OS_PowerSet(argv[value_idx]); // Set in SV task.
        return s;
    }
    IF_STATUS(s) { return s; }
    return OS_EnvVariableSet(argv[0], argv[value_idx]);
}

//------------------------------------------------------------------------------
static ConstStr cmd_unsetenv[]              = "unsetenv";
static ConstStr cmd_help_brief_unsetenv[]   = "Unset the environment variable.";
/******************************************************************************/
static Status OS_ShellCmdUnsetEnvHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdUnsetEnvHandler(const U32 argc, ConstStrPtr argv[])
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
OS_MemoryType mem_type = OS_MEM_UNDEF;
OS_MemoryStat mem_stat;

    printf("\n%-16s %-12s %-12s %-6s %-12s %-12s",
           "Name", "Address", "Size", "Block", "Used", "Free");
    while (OS_MEM_UNDEF != (mem_type = OS_MemoryTypeHeapNextGet(mem_type))) {
        IF_STATUS(OS_MemoryStatGet(mem_type, &mem_stat)) { return; }
        printf("\n%-16s 0x%-10X %-12d %-6d %-12d %-12d",
               mem_stat.desc.name_p,
               mem_stat.desc.addr,
               mem_stat.desc.size,
               mem_stat.desc.block_size,
               mem_stat.used,
               mem_stat.free);
    }
}

/******************************************************************************/
static void OS_ShellCmdStHandlerTskHelper(void);
void OS_ShellCmdStHandlerTskHelper(void)
{
const U32 task_inf_approx_mem_size = sizeof(OS_TaskStats);
register U32 tasks_count = OS_TasksCountGet();
OS_TaskStats* run_stats_buf_p = (OS_TaskStats*)OS_Malloc(task_inf_approx_mem_size * tasks_count);
const OS_TaskConfig cfg = { 0 };
OS_TaskStats* task_stats_p;
U32 uptime;

    if (OS_NULL == run_stats_buf_p) { return; }
    printf("\n%-12s %-3s %-4s %-3s %-4s %-7s %-10s %-4s %-5s %-4s %-3s %-3s",
           "Name", "TId", "PtId", "Pri", "PriP", "Power", "State", "CPU", "Stack", "Free", "In", "Out");
    if (tasks_count != OS_TasksStatsGet(run_stats_buf_p, tasks_count, &uptime)) { goto error; }
    uptime /= 100UL; //For percentage calculations.
    if (0 >= uptime) { goto error; } //Avoid divide by zero errors.
    task_stats_p = (OS_TaskStats*)&run_stats_buf_p[0];
    while (tasks_count--) {
        const OS_TaskHd thd = (OS_TaskHd)task_stats_p->xHandle;
        const OS_TaskHd par_thd = OS_TaskHdParentByHdGet(thd);
        const U32 par_id = (OS_NULL == par_thd) ? 0 : OS_TaskIdGet(par_thd);
        const OS_TaskConfig* cfg_p = OS_TaskConfigGet(thd);
        StrPtr cpu_str_buf[5];
        //TODO(A. Filyanov) Check for timer counter overflow! Otherwise -> wrong cpu usage value!
        snprintf((char*)&cpu_str_buf, sizeof(cpu_str_buf), "%d%%", (U32)(task_stats_p->ulRunTimeCounter / uptime));
        if (OS_NULL == cfg_p) {
            cfg_p = &cfg;
        }
        printf("\n%-12s %-3d %-4d %-3d %-4d %-7s %-10s %-4s %-5d %-4d %-3d %-3d",
               task_stats_p->pcTaskName,
               task_stats_p->xTaskNumber,
               par_id,
               task_stats_p->uxCurrentPriority,
               cfg_p->prio_power,
               OS_PowerStateNameGet(OS_TaskPowerStateGet(thd)),
               OS_TaskStateNameGet(OS_TaskStateGet(thd)),
               (const char*)cpu_str_buf,
               cfg_p->stack_size,
               task_stats_p->usStackHighWaterMark,
               cfg_p->stdin_len,
               cfg_p->stdout_len);
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

    printf("\n%-12s %-4s %-4s %-4s %-6s %-6s %-12s %-12s",
           "Parent", "PtId", "Dir", "Len", "ISize", "Items", "Sended", "Received");
    while (OS_NULL != (qhd = OS_QueueNextGet(qhd))) {
        OS_QueueConfig que_config;
        OS_QueueStats que_stats;
        IF_STATUS(OS_QueueConfigGet(qhd, &que_config)) { return; }
        IF_STATUS(OS_QueueStatsGet(qhd, &que_stats)) { return; }
        if (OS_NULL == (thd = OS_QueueParentGet(qhd))) {
            printf("\nTask undef!");
        } else {
            printf("\n%-12s %-4d %-4s %-4d %-6d %-6d %-12d %-12d",
                   OS_TaskNameGet(thd),
                   OS_TaskIdGet(thd),
                   (DIR_IN == que_config.dir) ? "in" : (DIR_OUT == que_config.dir) ? "out" : "Undef",
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

/******************************************************************************/
static void OS_ShellCmdStHandlerEvHelper(void);
void OS_ShellCmdStHandlerEvHelper(void)
{
OS_EventHd ehd = OS_NULL;

    printf("\n%-8s %-5s %-3s %-8s %-10s %-12s %-4s",
           "Name", "TimId", "Act", "Options", "Period", "Slot", "STId");
    while (OS_NULL != (ehd = OS_EventNextGet(ehd))) {
        OS_EventStats event_stats;
        IF_STATUS(OS_EventStatsGet(ehd, &event_stats)) { return; }
        const OS_TimerStats* timer_stats_p = &event_stats.timer_stats;
        const OS_TimerHd timer_hd = OS_TimerByIdGet(timer_stats_p->id);
        if (OS_NULL == timer_hd) { return; }
        printf("\n%-8s %-5d %-3s %-8d %-10d %-12s %-4d",
               timer_stats_p->name_p,
               timer_stats_p->id,
               (OS_TRUE == OS_TimerIsActive(timer_hd) ? "on" : "off"),
               timer_stats_p->options,
               timer_stats_p->period,
               OS_TaskNameGet(timer_stats_p->slot),
               OS_TaskIdGet(timer_stats_p->slot));
    }
}

/******************************************************************************/
static Status OS_ShellCmdStHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdStHandler(const U32 argc, ConstStrPtr argv[])
{
typedef struct {
    ConstStrPtr cmd;
    void (*handler)(void);
} CommandHandler;
CommandHandler cmd_handlers_v[] = {
    { "mem", OS_ShellCmdStHandlerMemHelper }, //memory
    { "tsk", OS_ShellCmdStHandlerTskHelper }, //tasks
    { "que", OS_ShellCmdStHandlerQueHelper }, //queues
    { "drv", OS_ShellCmdStHandlerDrvHelper }, //drivers
    { "tim", OS_ShellCmdStHandlerTimHelper }, //timers
    { "ev",  OS_ShellCmdStHandlerEvHelper  }, //events
//    { "fs",  OS_ShellCmdStHandlerFsHelper  }, //file system
//    { "net", OS_ShellCmdStHandlerNetHelper }, //network itf
    { OS_NULL }
};
    for (SIZE i = 0; OS_NULL != cmd_handlers_v[i].cmd; ++i) {
        if (!strcmp(cmd_handlers_v[i].cmd, (char const*)argv[0])) {
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
static Status OS_ShellCmdKillHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdKillHandler(const U32 argc, ConstStrPtr argv[])
{
Status s = S_OK;
    //Convert TaskId to number.
    const U8* tid_str = (const U8*)argv[0];
    U8 base = ('x' == *(U8*)(tid_str + 1)) ? 16 : 10;
    LNG tid = strtol((const char*)tid_str, OS_NULL, base);
    if (0 > tid) { return S_INVALID_REF; }

    const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_PWR, PWR_SHUTDOWN);
    const OS_TaskHd dst_thd = OS_TaskHdByIdGet(tid);
    if (OS_NULL != dst_thd) {
        const OS_QueueHd dst_stdin_qhd = OS_TaskStdIoGet(dst_thd, OS_STDIO_IN);
        if (OS_NULL != dst_stdin_qhd) {
            OS_SIGNAL_EMIT(dst_stdin_qhd, signal, OS_MSG_PRIO_NORMAL);
            const OS_QueueHd stdin_qhd = OS_TaskStdIoGet(OS_THIS_TASK, OS_STDIO_IN);
            OS_Message* msg_p;
            do {
                IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
                    OS_LOG_S(D_WARNING, S_UNDEF_MSG);
                    return S_UNDEF_MSG;
                } else {
                    if (OS_SIGNAL_IS(msg_p)) {
                        switch (OS_SIGNAL_ID_GET(msg_p)) {
                            case OS_SIG_PWR_ACK:
                                IF_STATUS(s = (Status)OS_SIGNAL_DATA_GET(msg_p)) {
                                }
                                s = OS_TaskDelete(dst_thd);
                                return s;
                                break;
                            case OS_SIG_STDOUT: //emited by OS_Log
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
            } while (OS_TRUE);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_time[]              = "time";
static ConstStr cmd_help_brief_time[]   = "Display current time.";
/******************************************************************************/
static Status OS_ShellCmdTimeHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdTimeHandler(const U32 argc, ConstStrPtr argv[])
{
Status s = S_OK;
    if (!strcmp("set", (char const*)argv[0])) {
        if ((2 < argc) || (1 > argc)) { return S_INVALID_ARGS_NUMBER; }
        OS_DateTime time = OS_TimeStringParse(argv[1]);
        s = OS_TimeSet(OS_TIME_UNDEF, &time);
    } else {
        const Locale locale = OS_LocaleGet();
        OS_DateTime time;
        s = OS_TimeGet(OS_TIME_LOCAL, &time);
        if (LOC_RU == locale) {
            printf("\n%02d" OS_LOCALE_TIME_DELIM_RU "%02d" OS_LOCALE_TIME_DELIM_RU "%02d", time.hour, time.min, time.sec);
        } else {
            printf("\n%02d" OS_LOCALE_TIME_DELIM_EN "%02d" OS_LOCALE_TIME_DELIM_EN "%02d", time.hour, time.min, time.sec);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_date[]              = "date";
static ConstStr cmd_help_brief_date[]   = "Display current date.";
/******************************************************************************/
static Status OS_ShellCmdDateHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdDateHandler(const U32 argc, ConstStrPtr argv[])
{
Status s = S_OK;
    if (!strcmp("set", (char const*)argv[0])) {
        if ((2 < argc) || (1 > argc)) { return S_INVALID_ARGS_NUMBER; }
        OS_DateTime date = OS_DateStringParse(argv[1]);
        s = OS_DateSet(OS_DATE_UNDEF, &date);
    } else {
        const Locale locale = OS_LocaleGet();
        OS_DateTime date;
        s = OS_DateGet(OS_DATE_UNDEF, &date);
        if (LOC_RU == locale) {
            printf("\n%-10s %02d" OS_LOCALE_DATE_DELIM_RU "%02d" OS_LOCALE_DATE_DELIM_RU "%04d",
                   OS_TimeNameDayOfWeekGet((OS_TimeWeekDay)date.wday, locale),
                   date.day, date.month, date.year);
        } else {
            printf("\n%-10s %02d" OS_LOCALE_DATE_DELIM_EN "%02d" OS_LOCALE_DATE_DELIM_EN "%04d",
                   OS_TimeNameDayOfWeekGet((OS_TimeWeekDay)date.wday, locale),
                   date.month, date.day, date.year);
        }
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_reboot[]            = "reboot";
static ConstStr cmd_help_brief_reboot[] = "Reboot the device.";
/******************************************************************************/
static Status OS_ShellCmdRebootHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdRebootHandler(const U32 argc, ConstStrPtr argv[])
{
const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_REBOOT, 0);
const OS_QueueHd sv_stdin_qhd = OS_TaskStdIoGet(OS_TaskHdParentGet(), OS_STDIO_IN);

    OS_SIGNAL_EMIT(OS_TaskSvcStdInGet(), signal, OS_MSG_PRIO_HIGH);
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr cmd_shutdown[]              = "shutdown";
static ConstStr cmd_help_brief_shutdown[]   = "Shutdown the device.";
/******************************************************************************/
static Status OS_ShellCmdShutdownHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdShutdownHandler(const U32 argc, ConstStrPtr argv[])
{
const OS_Signal signal = OS_SIGNAL_CREATE(OS_SIG_SHUTDOWN, 0);
const OS_QueueHd sv_stdin_qhd = OS_TaskStdIoGet(OS_TaskHdParentGet(), OS_STDIO_IN);

    OS_SIGNAL_EMIT(OS_TaskSvcStdInGet(), signal, OS_MSG_PRIO_HIGH);
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
    for (SIZE i = 0; i < ITEMS_COUNT_GET(cmd_cfg_std, OS_ShellCommandConfig); ++i) {
        const OS_ShellCommandConfig* cmd_cfg_p = &cmd_cfg_std[i];
        IF_STATUS(OS_ShellCommandCreate(cmd_cfg_p)) {
            OS_ASSERT(OS_FALSE);
        }
    }
    return S_OK;
}
