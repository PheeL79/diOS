/**************************************************************************//**
* @file    os_shell.h
* @brief   OS Shell.
* @author  A. Filyanov
* @warning Functions are only can be called from single instance of the shell task!
* @warning No thread safe, no stdio driver protection!
******************************************************************************/
#ifndef _OS_SHELL_H_
#define _OS_SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_common.h"

/**
* \defgroup OS_Shell OS_Shell
* @{
*/
//------------------------------------------------------------------------------
#define SHELL_COMMAND_UNDEF     OS_NULL

enum {
    OS_SHELL_BUTTON_UP              = 'A',
    OS_SHELL_BUTTON_DOWN            = 'B',
    OS_SHELL_BUTTON_RIGHT           = 'C',
    OS_SHELL_BUTTON_LEFT            = 'D',
    OS_SHELL_BUTTON_HOME            = '1',
    OS_SHELL_BUTTON_END             = '4',
    OS_SHELL_BUTTON_DELETE          = '3',

    OS_ASCII_EOL                    = '\0',
    OS_ASCII_SPACE                  = ' ',
    OS_ASCII_EXC_MARK               = '!',
    OS_ASCII_TILDE                  = '~',
    OS_ASCII_DOUBLE_QUOTE           = '\"',
    OS_ASCII_ESC_CR                 = '\r',
    OS_ASCII_ESC_LF                 = '\n',
    OS_ASCII_ESC_BACKSPACE          = 0x7F,
    OS_ASCII_ESC_NOT_ECHO_NEXT      = 0x1B,
    OS_ASCII_ESC_OP_SQUARE_BRAC     = 0x5B
};

//------------------------------------------------------------------------------
typedef Status (*OS_ShellCommandHandler)(const U32 argc, ConstStrP argv[]);

typedef enum {
    OS_SHELL_OPT_UNDEF
} OS_ShellOptions;

typedef struct {
    ConstStrP               command;
#ifdef OS_SHELL_HELP_ENABLED
    ConstStrP               help_brief;
    ConstStrP               help_detail;
#endif // OS_SHELL_HELP_ENABLED
    OS_ShellCommandHandler  handler;
    U8                      args_min;
    U8                      args_max;
    OS_ShellOptions         options;
} OS_ShellCommandConfig;
typedef OS_ShellCommandConfig* OS_ShellCommandHd;

//------------------------------------------------------------------------------
/// @brief      Initialise shell.
/// @return     #Status.
Status          OS_ShellInit(void);

/// @brief      Create shell command.
/// @param[in]  cmd_cfg_p       Shell command configuration.
/// @return     #Status.
Status          OS_ShellCommandCreate(const OS_ShellCommandConfig* cmd_cfg_p);

/// @brief      Delete shell command.
/// @param[in]  name_p          Shell command name.
/// @return     #Status.
Status          OS_ShellCommandDelete(ConstStrP name_p);

/// @brief      Execute current shell command.
/// @return     #Status.
Status          OS_ShellCommandExecute(void);

/// @brief      Check shell command arguments number.
/// @param[in]  cmd_hd          Shell command handler.
/// @param[in]  argc            Shell command arguments count.
/// @return     #Status.
Status          OS_ShellArgumentsNumberCheck(const OS_ShellCommandHd cmd_hd, const U8 argc);

/// @brief      Get shell command by it's name.
/// @param[in]  name_p          Shell command name.
/// @return     Shell command handler.
OS_ShellCommandHd OS_ShellCommandByNameGet(ConstStrP name_p);

/// @brief      Get the next shell command.
/// @param[in]  cmd_hd          Shell command handler.
/// @return     Driver handler.
OS_ShellCommandHd OS_ShellCommandNextGet(const OS_ShellCommandHd cmd_hd);

/// @brief      Get shell command prompt.
/// @return     Shell command prompt.
ConstStrP       OS_ShellPromptGet(void);

/// @brief      Clear shell buffer.
/// @return     #Status.
Status          OS_ShellCls(void);

/// @brief      Execute shell command line handler.
/// @param[in]  c               Command line input char.
/// @return     None.
void            OS_ShellClHandler(const U8 c);

/**@}*/ //OS_Shell

#ifdef __cplusplus
}
#endif

#endif // _OS_SHELL_H_
