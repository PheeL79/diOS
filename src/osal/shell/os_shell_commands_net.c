/**************************************************************************//**
* @file    os_shell_commands_net.c
* @brief   OS shell network commands.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "osal.h"
#include "os_shell_commands_net.h"
#include "os_shell.h"

//------------------------------------------------------------------------------
static ConstStr cmd_net[]      = "net";
static ConstStr cmd_help_net[] = "Display the message.";
/******************************************************************************/
static Status OS_ShellCmdNetHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdNetHandler(const U32 argc, ConstStrPtr argv[])
{
register U32 argc_count = 0;
    while (argc > argc_count) {
        printf("\n%s", argv[argc_count]);
        ++argc_count;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ShellCommandsNetInit(void)
{
ConstStr empty_str[] = "";
OS_ShellCommandConfig cmd_cfg;
    //Create and register network shell commands
    cmd_cfg.command     = cmd_net;
#ifdef OS_SHELL_HELP_ENABLED
    cmd_cfg.help_brief  = cmd_help_net;
    cmd_cfg.help_detail = empty_str;
#endif // OS_SHELL_HELP_ENABLED
    cmd_cfg.handler     = OS_ShellCmdNetHandler;
    cmd_cfg.options     = OS_SHELL_OPT_UNDEF;
    IF_STATUS(OS_ShellCommandCreate(&cmd_cfg)) {
        OS_ASSERT(OS_FALSE);
    }

    return S_OK;
}
