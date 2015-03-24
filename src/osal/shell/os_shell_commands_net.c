/**************************************************************************//**
* @file    os_shell_commands_net.c
* @brief   OS shell network commands.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include "hal.h"
#include "osal.h"
#include "os_shell_commands_net.h"
#include "os_shell.h"

#if (OS_NETWORK_ENABLED)
//------------------------------------------------------------------------------
static ConstStr cmd_netitf[]            = "netitf";
static ConstStr cmd_help_brief_netitf[] = "Setup network interface.";
/******************************************************************************/
static Status OS_ShellCmdNetItfHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdNetItfHandler(const U32 argc, ConstStrP argv[])
{
register U32 argc_count = 0;
    while (argc > argc_count) {
        printf("\n%s", argv[argc_count]);
        ++argc_count;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
static ConstStr empty_str[] = "";
static const OS_ShellCommandConfig cmd_cfg_net[] = {
    { cmd_netitf,   cmd_help_brief_netitf,  empty_str,              OS_ShellCmdNetItfHandler,   1,    3,      OS_SHELL_OPT_UNDEF  },
};

/******************************************************************************/
Status OS_ShellCommandsNetInit(void)
{
    //Create and register file system shell commands
    for (Size i = 0; i < ITEMS_COUNT_GET(cmd_cfg_net, OS_ShellCommandConfig); ++i) {
        const OS_ShellCommandConfig* cmd_cfg_p = &cmd_cfg_net[i];
        if (OS_NULL != cmd_cfg_p->command) {
            IF_STATUS(OS_ShellCommandCreate(cmd_cfg_p)) {
                OS_ASSERT(OS_FALSE);
            }
        }
    }
    return S_OK;
}

#endif //(OS_NETWORK_ENABLED)