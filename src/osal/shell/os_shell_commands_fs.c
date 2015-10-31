/**************************************************************************//**
* @file    os_shell_commands_fs.c
* @brief   OS shell file system commands definitions.
* @author  A. Filyanov
******************************************************************************/
#include <stdlib.h>
#include "hal.h"
#include "osal.h"
#include "os_memory.h"
#include "os_signal.h"
#include "os_mailbox.h"
#include "os_file_system.h"
#include "os_shell_commands_fs.h"
#include "os_shell.h"

#if (OS_FILE_SYSTEM_ENABLED)
//------------------------------------------------------------------------------
#undef  MDL_STATUS_ITEMS
#define MDL_STATUS_ITEMS        &status_fs_v[0]

//------------------------------------------------------------------------------
extern const StatusItem status_fs_v[];

//------------------------------------------------------------------------------
static OS_FileSystemMediaHd fs_media_hd_curr = OS_NULL;
static OS_FileHd fhd = OS_NULL;

//------------------------------------------------------------------------------
static ConstStr cmd_mi[]            = "mi";
static ConstStr cmd_help_brief_mi[] = "Disk media init.";
/******************************************************************************/
static Status OS_ShellCmdMiHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdMiHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    const S8 volume = OS_AtoI((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
    IF_STATUS(s = OS_FileSystemMediaInit(fs_media_hd, OS_NULL)) {
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_md[]            = "md";
static ConstStr cmd_help_brief_md[] = "Disk media deinit.";
/******************************************************************************/
static Status OS_ShellCmdMdHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdMdHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    const S8 volume = OS_AtoI((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
    IF_STATUS(s = OS_FileSystemMediaDeInit(fs_media_hd)) {
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fi[]            = "fi";
static ConstStr cmd_help_brief_fi[] = "File system init.";
/******************************************************************************/
static Status OS_ShellCmdFiHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFiHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    const S8 volume = OS_AtoI((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
    IF_STATUS(s = OS_FileSystemMount(fs_media_hd, OS_NULL)) {
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fd[]            = "fd";
static ConstStr cmd_help_brief_fd[] = "File system deinit.";
/******************************************************************************/
static Status OS_ShellCmdFdHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFdHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    const S8 volume = OS_AtoI((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
    IF_STATUS(s = OS_FileSystemUnMount(fs_media_hd)) {
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fs[]            = "fs";
static ConstStr cmd_help_brief_fs[] = "File system volume status.";
/******************************************************************************/
static Status OS_ShellCmdFsHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFsHandler(const U32 argc, ConstStrP argv[])
{
StrP path_p = OS_NULL;
OS_FileSystemVolumeStats stats_vol;
OS_FileSystemStats stats_fs;
S8 volume;
Status s = S_UNDEF;
    if (1 == argc) {
        //Parse volume string.
        char const delim_str[] = ":";
        char* path_str_p = (char*)argv[0];
        ConstStrP volume_label_p = (ConstStrP)OS_StrToK(path_str_p, delim_str);
        volume = OS_AtoI((const char*)volume_label_p);
        if (-1 == volume) {
            s = S_FS_MEDIA_INVALID;
            OS_LOG_S(L_WARNING, s);
            OS_LOG(L_WARNING, "Current media: %s", OS_FileSystemMediaNameGet(fs_media_hd_curr));
        } else {
            const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
            IF_STATUS(s = OS_FileSystemMediaCurrentSet(fs_media_hd)) { goto error; }
            fs_media_hd_curr = fs_media_hd;
        }
    }
    IF_STATUS(s = OS_FileSystemVolumeStatsGet(fs_media_hd_curr, &stats_vol)) { OS_LOG_S(L_WARNING, s); goto error; }
    path_p = (StrP)OS_Malloc(OS_FILE_SYSTEM_LONG_NAMES_LEN * 2);
    if (OS_NULL == path_p) { s = S_OUT_OF_MEMORY; goto error; }
    *path_p = OS_ASCII_EOL;
    OS_MemSet(&stats_fs, 0, sizeof(stats_fs));
    IF_STATUS(s = OS_FileSystemVolumeScan(path_p, &stats_fs)) { OS_LOG_S(L_WARNING, s); goto error; }
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
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fl[]            = "fl";
static ConstStr cmd_help_brief_fl[] = "Directory list.";
/******************************************************************************/
static Status OS_ShellCmdFlHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFlHandler(const U32 argc, ConstStrP argv[])
{
OS_DirHd dhd = OS_Malloc(sizeof(DIR));
OS_FileStats stats;
StrP path_p = "";
U32 dirs_count, files_count, total_size;
Status s = S_UNDEF;
    if (OS_NULL == dhd) { return S_OUT_OF_MEMORY; }
    if (1 == argc) {
        //Parse volume string.
        char const delim_str[] = ":";
        char* path_str_p = (char*)argv[0];
        ConstStrP volume_label_p = (ConstStrP)OS_StrToK(path_str_p, delim_str);
        const S8 volume = (S8)OS_AtoI((const char*)volume_label_p);
        if (-1 == volume) {
            s = S_FS_MEDIA_INVALID;
            OS_LOG_S(L_DEBUG_1, s);
            path_p = (StrP)argv[argc - 1];
        } else {
            const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
            IF_STATUS(s = OS_FileSystemMediaCurrentSet(fs_media_hd)) { goto error; }
            path_p = (StrP)(++path_str_p + OS_StrLen((const char*)volume_label_p));
            fs_media_hd_curr = fs_media_hd;
        }
    }
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    stats.long_name_size = OS_FILE_SYSTEM_LONG_NAMES_LEN;
    stats.long_name_p = OS_Malloc(stats.long_name_size);
    if (OS_NULL == stats.long_name_p) { s = S_OUT_OF_MEMORY; goto error; }
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
               stats.date_time.hours,
               stats.date_time.minutes,
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
    IF_OK(OS_FileSystemClustersFreeGet(fs_media_hd_curr, path_p, &total_size)) {
        OS_FileSystemVolumeStats volume_stats;
        IF_OK(s = OS_FileSystemVolumeStatsGet(fs_media_hd_curr, &volume_stats)) {
            printf(", %11u bytes free", (total_size * volume_stats.cluster_size));
        }
    }
error:
    OS_Free(dhd);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    OS_Free(stats.long_name_p);
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    IF_STATUS(s) { OS_LOG_S(L_WARNING, s); }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fk[]            = "fk";
static ConstStr cmd_help_brief_fk[] = "Directory create.";
/******************************************************************************/
static Status OS_ShellCmdFkHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFkHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    IF_STATUS(s = OS_DirectoryCreate(argv[0])) {
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fu[]            = "fu";
static ConstStr cmd_help_brief_fu[] = "Directory/file delete.";
/******************************************************************************/
static Status OS_ShellCmdFuHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFuHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    IF_STATUS(s = OS_DirectoryDelete(argv[0])) {
        OS_LOG_S(L_WARNING, s);
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
static Status OS_ShellCmdFnHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFnHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    IF_STATUS(s = OS_DirectoryRename(argv[0], argv[1])) {
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fa[]            = "fa";
static ConstStr cmd_help_brief_fa[] = "Directory/file attributes set.";
/******************************************************************************/
static Status OS_ShellCmdFaHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFaHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
OS_FileAttrs attrs = OS_FS_FILE_ATTR_UNDEF;
    { // Parse attributes
        StrP attrs_p = (StrP)argv[0];
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
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_ft[]            = "ft";
static ConstStr cmd_help_brief_ft[] = "Directory/file time stamp set.";
/******************************************************************************/
static Status OS_ShellCmdFtHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFtHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
    OS_DateTime date = OS_DateStringParse(argv[0]);
    if (OS_TRUE != OS_DateIsValid(date.year, date.month, date.day)) { return S_INVALID_VALUE; }
    OS_DateTime time = OS_TimeStringParse(argv[1]);
    date.hours  = time.hours;
    date.minutes= time.minutes;
    date.seconds= time.seconds;
    if (OS_TRUE != OS_TimeIsValid(date.hours, date.minutes, date.seconds)) { return S_INVALID_VALUE; }
    IF_STATUS(s = OS_FileDateTimeSet(argv[2], date)) {
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fo[]            = "fo";
static ConstStr cmd_help_brief_fo[] = "File open.";
//static ConstStr cmd_help_detail_fn[]= "Opens an object (file) and set it's mode. "\
/******************************************************************************/
static Status OS_ShellCmdFoHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFoHandler(const U32 argc, ConstStrP argv[])
{
Status s = S_UNDEF;
OS_FileOpenMode op_mode = OS_FS_FILE_OP_MODE_UNDEF;
U8 op = '\0';
Bool is_exists = OS_FALSE;
    { // Parse open mode
        StrP op_mode_p = (StrP)argv[0];
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
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

//------------------------------------------------------------------------------
static ConstStr cmd_fc[]            = "fc";
static ConstStr cmd_help_brief_fc[] = "File close.";
/******************************************************************************/
static Status OS_ShellCmdFcHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFcHandler(const U32 argc, ConstStrP argv[])
{
Status s = OS_FileClose(&fhd);
    IF_STATUS(s) {
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

#if (OS_FILE_SYSTEM_MAKE_ENABLED)
//------------------------------------------------------------------------------
static ConstStr cmd_fm[]            = "fm";
static ConstStr cmd_help_brief_fm[] = "File system make.";
/******************************************************************************/
static Status OS_ShellCmdFmHandler(const U32 argc, ConstStrP argv[]);
Status OS_ShellCmdFmHandler(const U32 argc, ConstStrP argv[])
{
Size size = 0;
OS_FileSystemPartitionRule part_rule = OS_FS_PART_RULE_UNDEF;
Status s = S_OK;
const S8 volume = OS_AtoI((const char*)argv[0]);
    if (-1 == volume) {
        s = S_FS_MEDIA_INVALID;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    //Parse partition rule string.
    char const delim_str[] = " ";
    char* part_rule_p = (char*)argv[1];
    const char* part_rule_str_p = OS_StrToK(part_rule_p, delim_str);
    if (!OS_StrCmp("fdisk", part_rule_str_p)) {
        part_rule = OS_FS_PART_RULE_FDISK;
    } else if (!OS_StrCmp("sfd", part_rule_str_p)) {
        part_rule = OS_FS_PART_RULE_SFD;
    } else {
        s = S_FS_INVALID_PARAMETER;
        OS_LOG_S(L_WARNING, s);
        return s;
    }
    if (3 == argc) {
        const U8* size_str = (const U8*)argv[3];
        const U8 base = ('x' == *(U8*)(size_str + 1)) ? 16 : 10;
        size = (Size)OS_StrToL((const char*)size_str, OS_NULL, base);
    }
    const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
    printf("\nThe %s will be formatted. Are you sure? (Y/n) ", OS_FileSystemMediaNameGet(fs_media_hd));
//-------------------------------------
// Intercept incoming user input(signals from STDIO driver) to task_shell. Workaround.
// TODO(A. Filyanov) Clarify application interface to allow users commands get an input(smthn like getc()).
    char c_buf[2];
    {
    char c;
    Size i = 0;
    const OS_QueueHd stdin_qhd = OS_TaskStdInGet(OS_THIS_TASK);
    OS_Message* msg_p;
        do {
            IF_STATUS(OS_MessageReceive(stdin_qhd, &msg_p, OS_BLOCK)) {
                    OS_LOG_S(L_WARNING, S_INVALID_MESSAGE);
            } else {
                if (OS_SignalIs(msg_p)) {
                    switch (OS_SignalIdGet(msg_p)) {
                        case OS_SIG_STDIN:
                            // get char from STDIO driver.
                            c = (char)OS_SignalDataGet(msg_p);
                            c_buf[i] = c;
                            i = (++i > ITEMS_COUNT_GET(c_buf, char)) ? --i : i;
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
        } while ((OS_ASCII_ESC_CR != c) || (OS_QueueItemsCountGet(stdin_qhd)));
    }
//-------------------------------------
    if ('Y' == c_buf[0]) {
        printf("Formating...");
        const OS_FileSystemMediaHd fs_media_hd = OS_FileSystemMediaByVolumeGet(volume);
        IF_STATUS(s = OS_FileSystemMake(fs_media_hd, part_rule, (Size)size)) {
            OS_LOG_S(L_WARNING, s);
        }
    }
    return s;
}
#endif // (OS_FILE_SYSTEM_MAKE_ENABLED)

//------------------------------------------------------------------------------
static ConstStr empty_str[] = "";
static const OS_ShellCommandConfig cmd_cfg_fs[] = {
    { cmd_mi,       cmd_help_brief_mi,      empty_str,              OS_ShellCmdMiHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_md,       cmd_help_brief_md,      empty_str,              OS_ShellCmdMdHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fi,       cmd_help_brief_fi,      empty_str,              OS_ShellCmdFiHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
    { cmd_fd,       cmd_help_brief_fd,      empty_str,              OS_ShellCmdFdHandler,       1,    1,      OS_SHELL_OPT_UNDEF  },
#if (OS_FILE_SYSTEM_MAKE_ENABLED)
    { cmd_fm,       cmd_help_brief_fm,      empty_str,              OS_ShellCmdFmHandler,       2,    3,      OS_SHELL_OPT_UNDEF  },
#endif // (OS_FILE_SYSTEM_MAKE_ENABLED)
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
    for (Size i = 0; i < ITEMS_COUNT_GET(cmd_cfg_fs, OS_ShellCommandConfig); ++i) {
        const OS_ShellCommandConfig* cmd_cfg_p = &cmd_cfg_fs[i];
        if (OS_NULL != cmd_cfg_p->command) {
            IF_STATUS(OS_ShellCommandCreate(cmd_cfg_p)) {
                OS_ASSERT(OS_FALSE);
            }
        }
    }
    return S_OK;
}

#endif //(OS_FILE_SYSTEM_ENABLED)