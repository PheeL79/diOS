/**************************************************************************//**
* @file    os_shell_commands_fs.c
* @brief   OS shell file system commands definitions.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "osal.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_message.h"
#include "os_file_system.h"
#include "os_shell_commands_fs.h"
#include "os_shell.h"

//------------------------------------------------------------------------------
#undef  MDL_STATUS_ITEMS
#define MDL_STATUS_ITEMS        &status_fs_v[0]

//------------------------------------------------------------------------------
extern const StatusItem status_fs_v[];

//------------------------------------------------------------------------------
static OS_FileHd fhd  = OS_FILE_UNDEF;
static S8 volume_curr = 0;

//------------------------------------------------------------------------------
static ConstStr cmd_mi[]            = "mi";
static ConstStr cmd_help_brief_mi[] = "Disk media init.";
/******************************************************************************/
static Status OS_ShellCmdMiHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdMiHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    const S8 volume = atoi((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    IF_STATUS(s = OS_FileSystemMediaInit(volume)) {
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_md[]            = "md";
static ConstStr cmd_help_brief_md[] = "Disk media deinit.";
/******************************************************************************/
static Status OS_ShellCmdMdHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdMdHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    const S8 volume = atoi((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    IF_STATUS(s = OS_FileSystemMediaDeInit(volume)) {
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fi[]            = "fi";
static ConstStr cmd_help_brief_fi[] = "File system init.";
/******************************************************************************/
static Status OS_ShellCmdFiHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFiHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    const S8 volume = atoi((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    IF_STATUS(s = OS_FileSystemMount(volume)) {
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fd[]            = "fd";
static ConstStr cmd_help_brief_fd[] = "File system deinit.";
/******************************************************************************/
static Status OS_ShellCmdFdHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFdHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    const S8 volume = atoi((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    IF_STATUS(s = OS_FileSystemUnMount(volume)) {
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fs[]            = "fs";
static ConstStr cmd_help_brief_fs[] = "File system volume status.";
/******************************************************************************/
static Status OS_ShellCmdFsHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFsHandler(const U32 argc, ConstStrPtr argv[])
{
StrPtr path_p = OS_NULL;
OS_VolumeStats stats_vol;
OS_FileSystemStats stats_fs;
S8 volume;
Status s;
    if (1 == argc) {
        //Parse volume string.
        char const delim_str[] = ":";
        char* path_str_p = (char*)argv[0];
        ConstStrPtr volume_label_p = (ConstStrPtr)strtok(path_str_p, delim_str);
        volume = atoi((const char*)volume_label_p);
        if (-1 == volume) {
            volume = volume_curr;
            s = S_FS_MEDIA_INVALID;
            OS_LOG_S(D_WARNING, s);
            OS_LOG(D_WARNING, "Current media: %s", OS_FileSystemMediaNameGet(volume));
        } else {
            IF_STATUS(s = OS_FileSystemMediaCurrentSet(volume)) { goto error; }
            volume_curr = volume;
        }
    }
    IF_STATUS(s = OS_FileSystemVolumeStatsGet(volume, &stats_vol)) { OS_LOG_S(D_WARNING, s); goto error; }
    path_p = (StrPtr)OS_Malloc(OS_FILE_SYSTEM_LONG_NAMES_LEN * 2);
    if (OS_NULL == path_p) { s = S_NO_MEMORY; goto error; }
    *path_p = OS_ASCII_EOL;
    memset(&stats_fs, 0, sizeof(stats_fs));
    IF_STATUS(s = OS_FileSystemVolumeScan(path_p, &stats_fs)) { OS_LOG_S(D_WARNING, s); goto error; }
    printf("\nMedia name            :%s"
           "\nVolume name           :%s"
           "\nVolume serial         :0x%X"
           "\nFAT type              :%s"
           "\nFAT sectors           :%u"
           "\nNumber of FATs        :%u"
           "\nRoot DIR entries      :%u"
           "\nNumber of clusters    :%u"
           "\nFree clusters         :%u"
           "\nVOL start (lba)       :%u"
           "\nFAT start (lba)       :%u"
           "\nDIR start (lba)       :%u"
           "\nDATA start (lba)      :%u",
           stats_vol.media_name_p,
           stats_vol.name,
           stats_vol.serial,
           (OS_FS_FAT12 == stats_vol.type) ? "FAT12" : (OS_FS_FAT16 == stats_vol.type) ? "FAT16" : "FAT32",
           stats_vol.fats_sectors,
           stats_vol.fats_count,
           stats_vol.root_dir_items_count,
           stats_vol.clusters_count,
           stats_vol.clusters_free,
           stats_vol.base_vol,
           stats_vol.base_fat,
           stats_vol.base_dir,
           stats_vol.base_data);

    printf("\n\n%u files, %u bytes.\n%u folders.\n"
           "%u KB total disk space.\n%u KB available.",
           stats_fs.files_count,
           stats_fs.files_total_size,
           stats_fs.dirs_count,
           (stats_vol.clusters_count * stats_vol.cluster_size) / 1024,
           (stats_vol.clusters_free  * stats_vol.cluster_size) / 1024);
error:
    OS_Free(path_p);
    OS_FileSystemMediaCurrentSet(volume_curr);
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fl[]            = "fl";
static ConstStr cmd_help_brief_fl[] = "Directory list.";
/******************************************************************************/
static Status OS_ShellCmdFlHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFlHandler(const U32 argc, ConstStrPtr argv[])
{
OS_DirHd dhd = OS_Malloc(sizeof(DIR));
OS_FileStats stats;
StrPtr path_p = "";
U32 dirs_count, files_count, total_size;
S8 volume = volume_curr;
Status s;
    if (OS_NULL == dhd) { return S_NO_MEMORY; }
    if (1 == argc) {
        //Parse volume string.
        char const delim_str[] = ":";
        char* path_str_p = (char*)argv[0];
        ConstStrPtr volume_label_p = (ConstStrPtr)strtok(path_str_p, delim_str);
        volume = (S8)atoi((const char*)volume_label_p);
        if (-1 == volume) {
            s = S_FS_MEDIA_INVALID;
            OS_LOG_S(D_DEBUG, s);
            path_p = (StrPtr)argv[argc - 1];
        } else {
            IF_STATUS(s = OS_FileSystemMediaCurrentSet(volume)) { goto error; }
            path_p = (StrPtr)(++path_str_p + strlen((const char*)volume_label_p));
            volume_curr = volume;
        }
    }
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    stats.long_name_size = OS_FILE_SYSTEM_LONG_NAMES_LEN;
    stats.long_name_p = OS_Malloc(stats.long_name_size);
    if (OS_NULL == stats.long_name_p) { s = S_NO_MEMORY; goto error; }
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    IF_STATUS(s = OS_DirectoryOpen(dhd, path_p)) { goto error; }
    dirs_count = files_count = total_size = 0;
    while (1) {
        IF_STATUS(s = OS_DirectoryRead(dhd, &stats)) { break; }
        if (OS_ASCII_EOL == stats.name[0]) { break; }
        if (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_DIR))) {
            ++dirs_count;
        } else {
            ++files_count;
            total_size += stats.size;
        }
        printf("\n%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %12s",
               (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_DIR))) ? 'd' : '-',
               (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_RDO))) ? 'r' : '-',
               (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_HID))) ? 'h' : '-',
               (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_SYS))) ? 's' : '-',
               (BIT_TEST(stats.attrs, BIT(OS_FS_FILE_ATTR_ARC))) ? 'a' : '-',
               stats.date_time.year,
               stats.date_time.day,
               stats.date_time.month,
               stats.date_time.hour,
               stats.date_time.min,
               stats.size,
               stats.name);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
        printf("  %s", stats.long_name_p);
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    }
    printf("\n\n%4u File(s),%11u bytes total\n%4u Dir(s)",
           files_count,
           total_size,
           dirs_count);
    IF_STATUS_OK(OS_FileSystemClustersFreeGet(volume, path_p, &total_size)) {
        OS_VolumeStats volume_stats;
        IF_STATUS_OK(s = OS_FileSystemVolumeStatsGet(volume, &volume_stats)) {
            printf(", %11u bytes free", (total_size * volume_stats.cluster_size));
        }
    }
error:
    OS_Free(dhd);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    OS_Free(stats.long_name_p);
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    OS_FileSystemMediaCurrentSet(volume_curr);
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fk[]            = "fk";
static ConstStr cmd_help_brief_fk[] = "Directory create.";
/******************************************************************************/
static Status OS_ShellCmdFkHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFkHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    IF_STATUS(s = OS_DirectoryCreate(argv[0])) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fu[]            = "fu";
static ConstStr cmd_help_brief_fu[] = "Directory/file delete.";
/******************************************************************************/
static Status OS_ShellCmdFuHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFuHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    IF_STATUS(s = OS_DirectoryDelete(argv[0])) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fn[]            = "fn";
static ConstStr cmd_help_brief_fn[] = "Directory/file rename.";
//static ConstStr cmd_help_detail_fn[]= "Renames an object (file or directory) and can also move it to other directory. "\
//"The logical drive number is determined by old name, new name must not contain a logical drive number. "\
//"Do not rename open objects or directry table can be collapted.";
/******************************************************************************/
static Status OS_ShellCmdFnHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFnHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    IF_STATUS(s = OS_DirectoryRename(argv[0], argv[1])) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fa[]            = "fa";
static ConstStr cmd_help_brief_fa[] = "Directory/file attributes set.";
/******************************************************************************/
static Status OS_ShellCmdFaHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFaHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
OS_FileAttrs attrs = OS_FS_FILE_ATTR_UNDEF;
    { // Parse attributes
        StrPtr attrs_p = (StrPtr)argv[0];
        while (OS_ASCII_EOL != *attrs_p) {
            switch (*attrs_p) {
                case 'a':
                    BIT_SET(attrs, BIT(OS_FS_FILE_ATTR_ARC));
                    break;
                case 'r':
                    BIT_SET(attrs, BIT(OS_FS_FILE_ATTR_RDO));
                    break;
                case 'h':
                    BIT_SET(attrs, BIT(OS_FS_FILE_ATTR_HID));
                    break;
                case 's':
                    BIT_SET(attrs, BIT(OS_FS_FILE_ATTR_SYS));
                    break;
                default:
                    break;
            }
            ++attrs_p;
        }
    }
    IF_STATUS(s = OS_FileAttributesSet(argv[1], attrs)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_ft[]            = "ft";
static ConstStr cmd_help_brief_ft[] = "Directory/file time stamp set.";
/******************************************************************************/
static Status OS_ShellCmdFtHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFtHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
    OS_DateTime date = OS_DateStringParse(argv[0]);
    if (OS_TRUE != OS_DateIsValid(date.year, date.month, date.day)) { return S_INVALID_VALUE; }
    OS_DateTime time = OS_TimeStringParse(argv[1]);
    date.hour = time.hour;
    date.min  = time.min;
    date.sec  = time.sec;
    if (OS_TRUE != OS_TimeIsValid(date.hour, date.min, date.sec)) { return S_INVALID_VALUE; }
    IF_STATUS(s = OS_FileDateTimeSet(argv[2], date)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fo[]            = "fo";
static ConstStr cmd_help_brief_fo[] = "File open.";
//static ConstStr cmd_help_detail_fn[]= "Opens an object (file) and set it's mode. "\
/******************************************************************************/
static Status OS_ShellCmdFoHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFoHandler(const U32 argc, ConstStrPtr argv[])
{
Status s;
OS_FileOpenMode op_mode = OS_FS_FILE_OP_MODE_UNDEF;
U8 op = '\0';
BL is_exists = OS_FALSE;
    { // Parse open mode
        StrPtr op_mode_p = (StrPtr)argv[0];
        while (OS_ASCII_EOL != *op_mode_p) {
            switch (*op_mode_p) {
                case 'c':
                    op = *op_mode_p;
                    BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_CREATE_NEW));
                    break;
                case 'o':
                    op = *op_mode_p;
                    BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_OPEN_NEW));
                    break;
                case 'r':
                    BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_READ));
                    break;
                case 'w':
                    BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_WRITE));
                    break;
                case 'x':
                    is_exists = OS_TRUE;
                    break;
                default:
                    break;
            }
            ++op_mode_p;
        }
    }
    if (OS_TRUE == is_exists) {
        if ('c' == op) {
            BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_CREATE_EXISTS));
        } else if ('o' == op) {
            BIT_SET(op_mode, BIT(OS_FS_FILE_OP_MODE_OPEN_EXISTS));
        }
    }
    IF_STATUS(s = OS_FileOpen(&fhd, argv[1], op_mode)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fc[]            = "fc";
static ConstStr cmd_help_brief_fc[] = "File close.";
/******************************************************************************/
static Status OS_ShellCmdFcHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFcHandler(const U32 argc, ConstStrPtr argv[])
{
Status s = OS_FileClose(&fhd);
    IF_STATUS(s) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

#if (1 == OS_FILE_SYSTEM_MAKE_ENABLED)
//------------------------------------------------------------------------------
static ConstStr cmd_fm[]            = "fm";
static ConstStr cmd_help_brief_fm[] = "File system make.";
/******************************************************************************/
static Status OS_ShellCmdFmHandler(const U32 argc, ConstStrPtr argv[]);
Status OS_ShellCmdFmHandler(const U32 argc, ConstStrPtr argv[])
{
U32 size = 0;
OS_FileSystemPartitionRule part_rule = OS_FS_PART_RULE_UNDEF;
Status s = S_OK;
const S8 volume = atoi((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    //Parse partition rule string.
    char const delim_str[] = " ";
    char* part_rule_p = (char*)argv[1];
    const char* part_rule_str_p = strtok(part_rule_p, delim_str);
    if (!strcmp("fdisk", part_rule_str_p)) {
        part_rule = OS_FS_PART_RULE_FDISK;
    } else if (!strcmp("sfd", part_rule_str_p)) {
        part_rule = OS_FS_PART_RULE_SFD;
    } else {
        s = S_FS_INVALID_PARAMETER;
        OS_LOG_S(D_WARNING, s);
        return s;
    }
    if (3 == argc) {
        const U8* size_str = (const U8*)argv[3];
        const U8 base = ('x' == *(U8*)(size_str + 1)) ? 16 : 10;
        size = (U32)strtol((const char*)size_str, OS_NULL, base);
    }
    printf("\nThe %s will be formatted. Are you sure? (Y/n) ", OS_FileSystemMediaNameGet(volume));
//-------------------------------------
// Intercept incoming user input(signals from STDIO driver) to task_shell. Workaround.
// TODO(A. Filyanov) Clarify application interface to allow users commands get an input(smthn like getc()).
    char c_buf[2];
    {
    char c;
    SIZE i = 0;
    const OS_QueueHd stdin_qhd = OS_TaskStdIoGet(OS_THIS_TASK, OS_STDIO_IN);
    OS_Message* msg_p;
        do {
            IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
                    OS_LOG_S(D_WARNING, S_UNDEF_MSG);
            } else {
                if (OS_SIGNAL_IS(msg_p)) {
                    switch (OS_SIGNAL_ID_GET(msg_p)) {
                        case OS_SIG_STDIN:
                            // get char from STDIO driver.
                            c = (char)OS_SIGNAL_DATA_GET(msg_p);
                            c_buf[i] = c;
                            i = (++i > ITEMS_COUNT_GET(c_buf, char)) ? --i : i;
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
        } while ((OS_ASCII_ESC_CR != c) || (OS_QueueItemsCountGet(stdin_qhd)));
    }
//-------------------------------------
    if ('Y' == c_buf[0]) {
        printf("Formating...");
        IF_STATUS(s = OS_FileSystemMake(volume, part_rule, (U32)size)) {
            OS_LOG_S(D_WARNING, s);
        }
    }
    return s;
}
#endif // (1 == OS_FILE_SYSTEM_MAKE_ENABLED)

//------------------------------------------------------------------------------
static ConstStr empty_str[] = "";
static const OS_ShellCommandConfig cmd_cfg_fs[] = {
    { cmd_mi,       cmd_help_brief_mi,      empty_str,              OS_ShellCmdMiHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_md,       cmd_help_brief_md,      empty_str,              OS_ShellCmdMdHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fi,       cmd_help_brief_fi,      empty_str,              OS_ShellCmdFiHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fd,       cmd_help_brief_fd,      empty_str,              OS_ShellCmdFdHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
#if (1 == OS_FILE_SYSTEM_MAKE_ENABLED)
    { cmd_fm,       cmd_help_brief_fm,      empty_str,              OS_ShellCmdFmHandler,       2,    3,      OS_SHELL_OPT_UNDEF  },
#endif // (1 == OS_FILE_SYSTEM_MAKE_ENABLED)
    { cmd_fs,       cmd_help_brief_fs,      empty_str,              OS_ShellCmdFsHandler,       0,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fo,       cmd_help_brief_fo,      empty_str,              OS_ShellCmdFoHandler,       2,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_fc,       cmd_help_brief_fc,      empty_str,              OS_ShellCmdFcHandler,       0,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fl,       cmd_help_brief_fl,      empty_str,              OS_ShellCmdFlHandler,       0,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fk,       cmd_help_brief_fk,      empty_str,              OS_ShellCmdFkHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fu,       cmd_help_brief_fu,      empty_str,              OS_ShellCmdFuHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fn,       cmd_help_brief_fn,      empty_str,              OS_ShellCmdFnHandler,       2,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_fa,       cmd_help_brief_fa,      empty_str,              OS_ShellCmdFaHandler,       2,    2,      OS_SHELL_OPT_UNDEF  },
    { cmd_ft,       cmd_help_brief_ft,      empty_str,              OS_ShellCmdFtHandler,       3,    3,      OS_SHELL_OPT_UNDEF  },
};

/******************************************************************************/
Status OS_ShellCommandsFsInit(void)
{
    //Create and register file system shell commands
    for (SIZE i = 0; i < ITEMS_COUNT_GET(cmd_cfg_fs, OS_ShellCommandConfig); ++i) {
        const OS_ShellCommandConfig* cmd_cfg_p = &cmd_cfg_fs[i];
        IF_STATUS(OS_ShellCommandCreate(cmd_cfg_p)) {
            OS_ASSERT(OS_FALSE);
        }
    }
    return S_OK;
}
