/**************************************************************************//**
* @file    os_shell.c
* @brief   OS Shell.
* @author  A. Filyanov
* @warning Functions are can only be called from single instance of the shell task!
* @warning No thread safe, no stdio driver protection!
******************************************************************************/
#include <string.h>
#include "osal.h"
#include "os_common.h"
#include "os_memory.h"
#include "os_list.h"
#include "os_task.h"
#include "os_debug.h"
#include "os_mutex.h"
#include "os_signal.h"
#include "os_message.h"
#include "os_shell_commands_std.h"
#if (1 == OS_FILE_SYSTEM_ENABLED)
#include "os_shell_commands_fs.h"
#endif // OS_FILE_SYSTEM_ENABLED
#include "os_shell.h"

/// @details Use macro D_TRACE\D_LOG there instead of OS_ equals.
///          (It's the same task as log, so no need to protect stdio output).

//------------------------------------------------------------------------------
#define OS_SHELL_PROMPT_CREATE(cr, prompt, space)  cr QUOTED(prompt) space

//------------------------------------------------------------------------------
static OS_List os_commands_list;
static OS_MutexHd os_shell_mutex;

//------------------------------------------------------------------------------
static ConstStr shell_prompt[] = OS_SHELL_PROMPT_CREATE("\n", OS_SHELL_PROMPT, " ");
static Str shell_cl[OS_SHELL_CL_LEN] = { OS_ASCII_EOL };
static S32 shell_cr_pos = 0;
static ConstStrP argv[OS_SHELL_CL_ARGS_MAX];

//------------------------------------------------------------------------------
#if (1 == OS_SHELL_EDIT_ENABLED)
static void OS_ShellClControlHandler(const U8 c);
static void OS_ShellCaretPositionSet(const S32 pos);
#endif // OS_SHELL_EDIT_ENABLED
static void OS_ShellClClear(void);
static void OS_ShellControlEcho(const U8 c);

/******************************************************************************/
#pragma inline
void OS_ShellClClear(void)
{
    shell_cr_pos = 0;
    //TODO(A. Filyanov) Remember cl cursor last max position and simply put EOL char at it.
    OS_MemSet(shell_cl, OS_ASCII_EOL, sizeof(shell_cl));
}

/******************************************************************************/
#pragma inline
void OS_ShellControlEcho(const U8 c)
{
    putchar(OS_ASCII_ESC_NOT_ECHO_NEXT);
    putchar(OS_ASCII_ESC_OP_SQUARE_BRAC);
    putchar(c);
}

/******************************************************************************/
#if (1 == OS_SHELL_EDIT_ENABLED)
#pragma inline
void OS_ShellCaretPositionSet(const S32 pos)
{
S32 caret_pos = pos - shell_cr_pos;
U8 caret_dir;
    shell_cr_pos += caret_pos;
    if (caret_pos < 0) {
        caret_dir = OS_SHELL_BUTTON_LEFT;
        caret_pos *= -1;
    } else {
        caret_dir = OS_SHELL_BUTTON_RIGHT;
    }
    while (caret_pos--) {
        OS_ShellControlEcho(caret_dir);
    }
}
#endif // OS_SHELL_EDIT_ENABLED

/******************************************************************************/
Status OS_ShellInit(void)
{
Status s = S_OK;
    os_shell_mutex = OS_MutexCreate();
    if (OS_NULL == os_shell_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_commands_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_commands_list)) { return S_INVALID_VALUE; }
    OS_ShellClClear();
    IF_STATUS(s = OS_ShellCommandsStdInit()) { return s; }
#if (1 == OS_FILE_SYSTEM_ENABLED)
    IF_STATUS(s = OS_ShellCommandsFsInit()) { return s; }
#endif // OS_FILE_SYSTEM_ENABLED
    return s;
}

/******************************************************************************/
Status OS_ShellCommandCreate(const OS_ShellCommandConfig* cmd_cfg_p)
{
const U32 cfg_size = sizeof(OS_ShellCommandConfig);
Status s = S_OK;

    if (OS_NULL == cmd_cfg_p) { return S_INVALID_REF; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_ShellCommandConfig* cmd_cfg_dyn_p = (OS_ShellCommandConfig*)OS_Malloc(cfg_size);
    if (OS_NULL == cmd_cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    IF_OK(s = OS_MutexLock(os_shell_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_MemCpy(cmd_cfg_dyn_p, cmd_cfg_p, cfg_size);
        OS_ListItemValueSet(item_l_p, (OS_Value)cmd_cfg_dyn_p);
        OS_ListItemOwnerSet(item_l_p, OS_TaskGet());
        OS_ListAppend(&os_commands_list, item_l_p);
        IF_STATUS(s) {
            OS_Free(cmd_cfg_dyn_p);
            OS_ListItemDelete(item_l_p);
        }
        OS_MutexUnlock(os_shell_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_ShellCommandDelete(ConstStrP name_p)
{
OS_ShellCommandConfig* cmd_cfg_p = OS_ShellCommandByNameGet(name_p);
OS_ListItem* item_l_p = OS_ListItemByValueGet(&os_commands_list, (OS_Value)cmd_cfg_p);
Status s = S_OK;

    if ((OS_NULL == cmd_cfg_p) || (OS_DELAY_MAX == (OS_Value)cmd_cfg_p)) { return S_INVALID_REF; }
    IF_OK(s = OS_MutexLock(os_shell_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItemDelete(item_l_p);
        OS_Free(cmd_cfg_p);
        OS_MutexUnlock(os_shell_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_ShellCommandExecute(void)
{
//TODO(A. Filyanov) OS_STRTOK()?;
U32 argc = 0;
StrP cl_p = shell_cl;
U8 c =* cl_p;
Bool is_quoted = OS_FALSE;
    //Parse command line.
    while (OS_ASCII_EOL != c) {
        if (OS_ASCII_DOUBLE_QUOTE == c) {
            if (OS_FALSE == is_quoted) {
                is_quoted = OS_TRUE;
                *cl_p = OS_ASCII_EOL;
                argv[argc] = cl_p + 1;
                ++argc;
            } else {
                is_quoted = OS_FALSE;
                *cl_p = OS_ASCII_EOL;
            }
        } else if (OS_ASCII_SPACE == c) {
            if (OS_FALSE == is_quoted) {
                const U8 next_c = *(cl_p + 1);
                *cl_p = OS_ASCII_EOL;
                if ((OS_ASCII_SPACE != next_c) && (OS_ASCII_DOUBLE_QUOTE != next_c)) {
                    argv[argc] = cl_p + 1;
                    ++argc;
                }
            }
        }
        c = *(++cl_p);
    }
    //Execute the command handler.
    const OS_ShellCommandHd cmd_hd = OS_ShellCommandByNameGet(shell_cl);
    if (SHELL_COMMAND_UNDEF == cmd_hd) {
        return S_UNDEF_CMD;
    }
    IF_STATUS(OS_ShellArgumentsNumberCheck(cmd_hd, argc)) {
        return S_INVALID_ARGS_NUMBER;
    }
    return cmd_hd->handler(argc, argv);
}

/******************************************************************************/
OS_ShellCommandHd OS_ShellCommandByNameGet(ConstStrP name_p)
{
OS_ShellCommandHd cmd_hd = SHELL_COMMAND_UNDEF;

    IF_OK(OS_MutexLock(os_shell_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_commands_list));
        OS_ShellCommandConfig* cmd_cfg_p;

        while (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
            cmd_cfg_p = (OS_ShellCommandConfig*)OS_ListItemValueGet(iter_li_p);
            if (!OS_StrCmp((const char*)name_p, (const char*)cmd_cfg_p->command)) {
                cmd_hd = (OS_ShellCommandHd)OS_ListItemValueGet(iter_li_p);
                break;
            }
            iter_li_p = OS_ListItemNextGet(iter_li_p);
        }
        OS_MutexUnlock(os_shell_mutex);
    }
    return cmd_hd;
}

/******************************************************************************/
OS_ShellCommandHd OS_ShellCommandNextGet(const OS_ShellCommandHd cmd_hd)
{
OS_ListItem* iter_li_p;
OS_ShellCommandHd chd = SHELL_COMMAND_UNDEF;
    IF_OK(OS_MutexLock(os_shell_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        if (SHELL_COMMAND_UNDEF == cmd_hd) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_commands_list));
            if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) { goto error; }
            chd = (OS_ShellCommandHd)OS_ListItemValueGet(iter_li_p);
        } else {
            iter_li_p = OS_ListItemByValueGet(&os_commands_list, (OS_Value)cmd_hd);
            if (OS_NULL != iter_li_p) {
                iter_li_p = OS_ListItemNextGet(iter_li_p);
                if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) { goto error; }
                chd = (OS_ShellCommandHd)OS_ListItemValueGet(iter_li_p);
            }
        }
error:
        OS_MutexUnlock(os_shell_mutex);
    }
    return chd;
}

/******************************************************************************/
Status OS_ShellArgumentsNumberCheck(const OS_ShellCommandHd cmd_hd, const U8 argc)
{
    if ((cmd_hd->args_min <= argc) && (cmd_hd->args_max >= argc)) { return S_OK; }
    return S_INVALID_ARGS_NUMBER;
}

/******************************************************************************/
ConstStrP OS_ShellPromptGet(void)
{
    return shell_prompt;
}

/******************************************************************************/
Status OS_ShellCls(void)
{
    HAL_StdIoCls();
    return S_OK;
}

/******************************************************************************/
void OS_ShellClHandler(const U8 c)
{
#if (1 == OS_SHELL_EDIT_ENABLED)
static BL is_echo = OS_OS_TRUE;
static BL is_ctrl = OS_FALSE;
#endif // OS_SHELL_EDIT_ENABLED
    switch (c) {
        case OS_ASCII_ESC_CR:
        case OS_ASCII_ESC_LF: {
#if (0 == OS_SHELL_EDIT_ENABLED)
            OS_ShellControlEcho(OS_SHELL_BUTTON_UP); //Cursor up to cancel remote terminal LF.
#endif // OS_SHELL_EDIT_ENABLED
            const Status s = OS_ShellCommandExecute();
            const OS_LogLevel level = (S_OK != s) ? D_WARNING : D_INFO;
            OS_ShellClClear();
            OS_LOG_S(level, s);
            }
            break;
#if (1 == OS_SHELL_EDIT_ENABLED)
        case OS_ASCII_SPACE: {
            const U32 str_len = OS_StrLen((char const*)&shell_cl[shell_cr_pos]) + 1;
            const StrP cl_str_p = &shell_cl[shell_cr_pos];
            if ((shell_cr_pos + str_len + 1) < sizeof(shell_cl)) { //protect cl buffer
                OS_MemMov(cl_str_p + 1, cl_str_p, str_len + 1);
                *cl_str_p = OS_ASCII_SPACE;
                putchar(c); //echo
                printf((char const*)cl_str_p + 1);
                const S32 pos = shell_cr_pos + 1;
                shell_cr_pos += str_len;
                //Return terminal caret to position.
                OS_ShellCaretPositionSet(pos);
            }
            }
            break;
        case OS_ASCII_ESC_BACKSPACE:
            if (0 < shell_cr_pos) {
                const U32 str_len = OS_StrLen((char const*)&shell_cl[shell_cr_pos]);
                const StrP cl_str_p = &shell_cl[shell_cr_pos];
                OS_MemMov(cl_str_p - 1, cl_str_p, str_len + 1);
                putchar(c); //echo
                printf("%s%c", (char const*)cl_str_p - 1, OS_ASCII_SPACE);
                const S32 pos = --shell_cr_pos;
                shell_cr_pos += str_len + 1;
                //Return terminal caret to position.
                OS_ShellCaretPositionSet(pos);
            }
            break;
        case OS_ASCII_ESC_NOT_ECHO_NEXT:
            is_echo = OS_FALSE;
            break;
        case OS_ASCII_ESC_OP_SQUARE_BRAC:
            if (OS_FALSE == is_echo) {
                is_ctrl = OS_OS_TRUE;
                is_echo = OS_OS_TRUE;
            } else {
                goto echo; //echo
            }
            break;
        case OS_ASCII_TILDE:
            if (OS_OS_TRUE == is_ctrl) {
                is_ctrl = OS_FALSE;
            } else {
                goto echo; //echo
            }
            break;
        case OS_SHELL_BUTTON_HOME:
        case OS_SHELL_BUTTON_END:
        case OS_SHELL_BUTTON_DELETE:
            if (OS_TRUE == is_ctrl) {
                OS_ShellClControlHandler(c);
            } else {
                goto echo;
            }
            break;
        case OS_SHELL_BUTTON_LEFT:
        case OS_SHELL_BUTTON_RIGHT:
        case OS_SHELL_BUTTON_UP:
        case OS_SHELL_BUTTON_DOWN:
            if (OS_TRUE == is_ctrl) {
                OS_ShellClControlHandler(c);
                is_ctrl = OS_FALSE;
            } else {
                goto echo;
            }
            break;
//        case OS_ASCII_ESC_PAGE_UP:
//        case OS_ASCII_ESC_PAGE_DOWN:
//        case OS_ASCII_ESC_INSERT:
//            break;
#endif // OS_SHELL_EDIT_ENABLED
        default: //not an escape symbol
#if (1 == OS_SHELL_EDIT_ENABLED)
echo:
            {
            const U32 str_len = OS_StrLen((char const*)&shell_cl[shell_cr_pos]) + 1;
            const StrP cl_str_p = &shell_cl[shell_cr_pos];
            if ((shell_cr_pos + str_len + 1) < sizeof(shell_cl)) { //protect cl buffer
                OS_MemMov(cl_str_p + 1, cl_str_p, str_len + 1);
                *cl_str_p = c;
                putchar(c); //echo
                printf((char const*)cl_str_p + 1);
                const S32 pos = shell_cr_pos + 1;
                shell_cr_pos += str_len;
                //Return terminal caret to position.
                OS_ShellCaretPositionSet(pos);
            }
            }
#else
            if (shell_cr_pos < sizeof(shell_cl)) { //protect cl buffer
                shell_cl[shell_cr_pos++] = c;
            }
#endif // OS_SHELL_EDIT_ENABLED
            break;
    }
}

/******************************************************************************/
#if (1 == OS_SHELL_EDIT_ENABLED)
void OS_ShellClControlHandler(const U8 c)
{
    switch (c) {
        case OS_SHELL_BUTTON_HOME:
            OS_ShellCaretPositionSet(0);
            break;
        case OS_SHELL_BUTTON_END:
            OS_ShellCaretPositionSet(OS_StrLen((char const*)&shell_cl));
            break;
        case OS_SHELL_BUTTON_DELETE: {
            const S32 str_len = OS_StrLen((char const*)&shell_cl[shell_cr_pos]);
            if (str_len) {
                const StrP cl_str_p = &shell_cl[shell_cr_pos];
                OS_MemMov(cl_str_p, cl_str_p + 1, str_len + 1);
                printf("%s%c", (char const*)cl_str_p, OS_ASCII_SPACE);
                const S32 pos = shell_cr_pos;
                shell_cr_pos += str_len;
                //Return terminal caret to position.
                OS_ShellCaretPositionSet(pos);
            }
            }
            break;
        case OS_SHELL_BUTTON_LEFT:
            if (0 < shell_cr_pos) {
                OS_ShellControlEcho(c);
                --shell_cr_pos;
                if (OS_ASCII_EOL == shell_cl[shell_cr_pos]) {
                    shell_cl[shell_cr_pos] = OS_ASCII_SPACE;
                }
            }
            break;
        case OS_SHELL_BUTTON_RIGHT:
            if (sizeof(shell_cl) > shell_cr_pos) {
                OS_ShellControlEcho(c);
                if (OS_ASCII_EOL == shell_cl[shell_cr_pos]) {
                    shell_cl[shell_cr_pos] = OS_ASCII_SPACE;
                }
                ++shell_cr_pos;
            }
            break;
        case OS_SHELL_BUTTON_UP:
        case OS_SHELL_BUTTON_DOWN:
            //ignore
            //TODO(A. Filyanov) Navigate the recent command entries.
            break;
        default:
            break;
    }
}
#endif // OS_SHELL_EDIT_ENABLED