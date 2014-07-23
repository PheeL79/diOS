/***************************************************************************//**
* @file    os_file_system.c
* @brief   OS File system.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "sdcard.h"
#include "stm324xg_eval_sdio_sd.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_driver.h"
#include "os_time.h"
#include "os_file_system.h"

#if (1 == OS_FILE_SYSTEM_ENABLED)

//-----------------------------------------------------------------------------
#define MDL_NAME            "file_system"
#undef  MDL_STATUS_ITEMS
#define MDL_STATUS_ITEMS    &status_fs_v[0]

//------------------------------------------------------------------------------
enum {
    ATA = 0,
    MMC = 1,
    USB = 2
};

typedef struct {
    StrPtr  logical_drive_p;
    StrPtr  name_p;
} OS_FileSystemMediaLabel;

//------------------------------------------------------------------------------
//static void FDirTranslate(const DIR* dir_p, OS_DirHd dhd);
static OS_DateTime  FDateTimeTranslate(const WORD date, const WORD time);
static OS_FileAttrs FAttributeTranslate(const BYTE a);
static BYTE         FOpenModeConvert(const OS_FileOpenMode op_mode);
static DWORD        FDateTimeConvert(const OS_DateTime date_time);
static BYTE         FAttributesConvert(const OS_FileAttrs attrs);
static Status       FResultTranslate(const FRESULT r);
static Status       VolumeCheck(const S8 volume);

//------------------------------------------------------------------------------
static OS_DriverHd drv_led_fs;

static OS_DriverHd drv_media_v[OS_FILE_SYSTEM_VOLUMES_MAX];
static OS_FileSystemHd fshd_v[OS_FILE_SYSTEM_VOLUMES_MAX];
static OS_FileSystemMediaLabel media_label_v[OS_FILE_SYSTEM_VOLUMES_MAX] = {
    { "0:", "ATA"       },
    { "1:", "SD Card"   },
    { "2:", "USB"       }
};

const StatusItem status_fs_v[] = {
//file system
    {"Undefined status"},
    {"Unmounted"},
    {"Isn't inited"},
    {"Media fault"},
    {"Assert"},
    {"Not ready"},
    {"File not found"},
    {"Path not found"},
    {"Invalid file name"},
    {"Access denied"},
    {"Object exists"},
    {"Invalid object"},
    {"Write protected"},
    {"Invalid media"},
    {"Not enabled"},
    {"No file system"},
    {"Make file system aborted"},
    {"Timeout"},
    {"Locked"},
    {"Allocation fail"},
    {"Too many opened files"},
    {"Invalid parameter"},
    {"I/O operation fail"},
    {"End of file"}
};

/******************************************************************************/
Status OS_FileSystemInit(void)
{
Status s;
    D_LOG(D_INFO, "Init");
    memset(&drv_media_v[0], 0, sizeof(drv_media_v));
    //Led SD Init/Open
    const OS_DriverConfig drv_cfg = {
        .name       = "LED_FS",
        .itf_p      = drv_led_v[DRV_ID_LED_FS],
        .prio_power = OS_PWR_PRIO_DEFAULT
    };
    IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&drv_led_fs)) { return s; }
    IF_STATUS(s = OS_DriverInit(drv_led_fs)) { return s; }
    IF_STATUS(s = OS_DriverOpen(drv_led_fs, OS_NULL)) { return s; }
    return s;
}

/******************************************************************************/
Status OS_FileSystemDeInit(void)
{
Status s;
    //Led FS Close/Deinit
    IF_STATUS(s = OS_DriverClose(drv_led_fs)) {
        OS_LOG_S(D_WARNING, s);
    }
    IF_STATUS(s = OS_DriverDeInit(drv_led_fs)) {
        OS_LOG_S(D_WARNING, s);
    }
    return S_OK;
}

/******************************************************************************/
Status OS_FileSystemMediaInit(const S8 volume)
{
OS_DriverHd* drv_media_p = &drv_media_v[volume];
OS_DriverStats stats;
Status s = S_OK;
    OS_LOG(D_DEBUG, "FS media init: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    if (OS_NULL == *drv_media_p) {
        OS_FileSystemHd fshd_p = OS_Malloc(sizeof(FATFS));
        if (OS_NULL == fshd_p) { return S_NO_MEMORY; }
        fshd_v[volume] = fshd_p;
        const OS_DriverConfig drv_cfg = {
            .name       = "SDIO",
            .itf_p      = drv_sdio_v[DRV_ID_SDIO],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)drv_media_p)) { return s; }
    }
    IF_STATUS(s = OS_DriverStatsGet(*drv_media_p, &stats)) { return s; }
    if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_INIT))) {
        IF_STATUS(s = OS_DriverInit(*drv_media_p)) { return s; }
    }
    if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverOpen(*drv_media_p, &drv_led_fs)) { return s; }
    }
    const DSTATUS dstatus = disk_initialize(volume);
    if (BIT_TEST(dstatus, STA_NODISK)) {
        s = S_FS_UNMOUNTED;
    } else if (BIT_TEST(dstatus, STA_NOINIT)) {
        s = S_FS_ISNT_INITED;
    } else if (BIT_TEST(dstatus, STA_PROTECT)) {
        s = S_FS_WRITE_PROTECTED;
    }
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaDeInit(const S8 volume)
{
OS_DriverHd* drv_media_p = &drv_media_v[volume];
Status s;
    OS_LOG(D_DEBUG, "FS media deinit: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    IF_STATUS(s = OS_DriverIoCtl(*drv_media_p, DRV_REQ_STD_SYNC, OS_NULL)) {
        OS_LOG_S(D_WARNING, s);
    }
    if (OS_NULL != *drv_media_p) {
        OS_FileSystemUnMount(volume);
    }
    IF_STATUS(s = OS_DriverClose(*drv_media_p))  {
        OS_LOG_S(D_WARNING, s);
    }
    IF_STATUS(s = OS_DriverDeInit(*drv_media_p)) {
        OS_LOG_S(D_WARNING, s);
    }
    IF_STATUS(s = OS_DriverDelete(*drv_media_p)) {
        OS_LOG_S(D_WARNING, s);
    }
    OS_Free(fshd_v[volume]);
    *drv_media_p = OS_NULL;
    return s;
}

/******************************************************************************/
//S8 OS_FileSystemMediaCurrentGet(void)
//{
//    return volume_curr;
//}

/******************************************************************************/
Status OS_FileSystemMediaCurrentSet(const S8 volume)
{
Status s;
    OS_LOG(D_DEBUG, "FS media set: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    IF_STATUS_OK(s = FResultTranslate(f_chdrive((const char*)media_label_v[volume].logical_drive_p))) {
//        volume_curr = volume;
    }
    return s;
}

/******************************************************************************/
StrPtr OS_FileSystemMediaNameGet(const S8 volume)
{
    IF_STATUS(VolumeCheck(volume)) { return OS_NULL; }
    return media_label_v[volume].name_p;
}

#if (1 == OS_FILE_SYSTEM_MAKE_ENABLED)
/******************************************************************************/
Status OS_FileSystemMake(const S8 volume, const OS_FileSystemPartitionRule part_rule, const U32 size)
{
Status s;
const BYTE fpart_rule = (OS_FS_PART_RULE_FDISK == part_rule) ? 0 :
                        (OS_FS_PART_RULE_SFD   == part_rule) ? 1 : U8_MAX;
    OS_LOG(D_DEBUG, "FS make: %s, rule: %d, size: %u", OS_FileSystemMediaNameGet(volume), part_rule, size);
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    if (U8_MAX == fpart_rule) { return S_FS_INVALID_PARAMETER; }
    return FResultTranslate(f_mkfs((const char*)media_label_v[volume].logical_drive_p, fpart_rule, size));
}
#endif // (1 == OS_FILE_SYSTEM_MAKE_ENABLED)

/******************************************************************************/
Status OS_FileSystemMount(const S8 volume)
{
Status s;
    OS_LOG(D_DEBUG, "FS mount volume: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
#ifndef NDEBUG
    memset(fshd_v[1], 0xF5, sizeof(FATFS));
#endif // NDEBUG
    return FResultTranslate(f_mount((FATFS*)fshd_v[volume], (const char*)media_label_v[volume].logical_drive_p, 1));
}

/******************************************************************************/
Status OS_FileSystemUnMount(const S8 volume)
{
Status s;
    OS_LOG(D_DEBUG, "FS unmount volume: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    return FResultTranslate(f_mount(OS_NULL, (const char*)media_label_v[volume].logical_drive_p, 1));
}

/******************************************************************************/
S8 OS_FileSystemVolumeByNameGet(ConstStrPtr name_p)
{
    for (S8 i = 0; i < OS_FILE_SYSTEM_VOLUMES_MAX; ++i) {
        if (!strcmp((const char*)name_p, (const char*)media_label_v[i].name_p)) {
            return i;
        }
    }
    return -1;
}

/******************************************************************************/
Status OS_FileSystemVolumeLabelGet(const S8 volume, StrPtr label_p, U32* serial_p)
{
Status s;
    OS_LOG(D_DEBUG, "FS vol label get: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    return FResultTranslate(f_getlabel((const char*)media_label_v[volume].logical_drive_p,
                                       (char*)label_p,
                                       (DWORD*)serial_p));
}

/******************************************************************************/
Status OS_FileSystemVolumeLabelSet(const S8 volume, StrPtr label_p)
{
#define VOLUME_LOG_DRV_LEN  02U
#define VOLUME_NAME_LEN     11U
Str vol_label[VOLUME_LOG_DRV_LEN + VOLUME_NAME_LEN];
Status s;
    OS_LOG(D_DEBUG, "FS vol label set: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    strncpy((char*)&vol_label[0], (char const*)media_label_v[volume].logical_drive_p, VOLUME_LOG_DRV_LEN);
    strncpy((char*)&vol_label[VOLUME_LOG_DRV_LEN], (char const*)label_p, VOLUME_NAME_LEN);
    return FResultTranslate(f_setlabel((const char*)vol_label));
}

/******************************************************************************/
Status OS_FileSystemVolumeStatsGet(const S8 volume, OS_VolumeStats* stats_p)
{
Status s = S_OK;
    OS_LOG(D_DEBUG, "FS vol stats get: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    if (OS_NULL == stats_p) { return S_INVALID_REF; }
    FATFS* fs_p = (FATFS*)fshd_v[volume];
    memset(stats_p, 0x0, sizeof(OS_VolumeStats));
    {
    DWORD clusters_count;
        IF_STATUS(s = FResultTranslate(f_chdrive((const char*)media_label_v[volume].logical_drive_p))) { return s; }
        IF_STATUS(s = FResultTranslate(f_getfree((const char*)media_label_v[volume].logical_drive_p,
                                                &clusters_count, &fs_p))) { return s; }
        IF_STATUS(s = OS_FileSystemVolumeLabelGet(volume,
                                                 (StrPtr)stats_p->name,
                                                 &stats_p->serial)) { return s; }
    }
    stats_p->media_name_p           = media_label_v[volume].name_p;
    stats_p->type                   = (FS_FAT12 == fs_p->fs_type) ? OS_FS_FAT12 :
                                      (FS_FAT16 == fs_p->fs_type) ? OS_FS_FAT16 :
                                      (FS_FAT32 == fs_p->fs_type) ? OS_FS_FAT32 : OS_FS_UNDEF;
    stats_p->cluster_size           = fs_p->csize * SD_CARD_SECTOR_SIZE;
    stats_p->clusters_count         = fs_p->n_fatent - 2;
    stats_p->clusters_free          = fs_p->free_clust;
    stats_p->root_dir_items_count   = fs_p->n_rootdir;
    stats_p->fats_count             = fs_p->n_fats;
    stats_p->fats_sectors           = fs_p->fsize;
    stats_p->base_vol               = fs_p->volbase;
    stats_p->base_fat               = fs_p->fatbase;
    stats_p->base_dir               = fs_p->dirbase;
    stats_p->base_data              = fs_p->database;
    return s;
}

/******************************************************************************/
Status OS_FileSystemClustersFreeGet(const S8 volume, const StrPtr path_p, U32* clusters_free_count_p)
{
Status s;
    OS_LOG(D_DEBUG, "FS clusters free: %s", OS_FileSystemMediaNameGet(volume));
    IF_STATUS(s = VolumeCheck(volume)) { return s; }
    return FResultTranslate(f_getfree((const char*)path_p, (DWORD*)clusters_free_count_p, (FATFS**)&fshd_v[volume]));
}

/******************************************************************************/
Status OS_FileSystemVolumeScan(const StrPtr path_p, OS_FileSystemStats* stats_p)
{
OS_DirHd dir_hd;
OS_FileStats stats_file;
U32 path_len;
StrPtr file_name_p;
Status s = S_OK;
    OS_LOG(D_DEBUG, "FS volume scan: %s", path_p);
    if (OS_NULL == stats_p) { return S_INVALID_REF; }
    dir_hd = OS_Malloc(sizeof(DIR));
    if (OS_NULL == dir_hd) { return S_NO_MEMORY; }
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    stats_file.long_name_size = OS_FILE_SYSTEM_LONG_NAMES_LEN;
    stats_file.long_name_p = OS_Malloc(stats_file.long_name_size);
    if (OS_NULL == stats_file.long_name_p) { s = S_NO_MEMORY; goto error; }
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    IF_STATUS(s = OS_DirectoryOpen(dir_hd, path_p)) { s = S_FS_PATH_NOT_FOUND; goto error; }
    path_len = strlen((const char*)path_p);
    while ((S_OK == (s = OS_DirectoryRead(dir_hd, &stats_file))) && (OS_ASCII_EOL != stats_file.name[0])) {
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
        file_name_p = (*stats_file.long_name_p) ? stats_file.long_name_p : stats_file.name;
#else
        file_name_p = stats_file.name;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
        if (BIT_TEST(stats_file.attrs, BIT(OS_FS_FILE_ATTR_DIR))) {
            // Guard relative directories.
            if (OS_NULL != strchr((const char*)file_name_p, '.')) { continue; }
            stats_p->dirs_count++;
            *(path_p + path_len) = OS_FILE_SYSTEM_DIR_DELIM;
            strcpy((char*)(path_p + path_len + 1), (const char*)file_name_p);
            s = OS_FileSystemVolumeScan(path_p, stats_p);
            *(path_p + path_len) = OS_ASCII_EOL;
            IF_STATUS(s) { break; }
        } else {
            stats_p->files_count++;
            stats_p->files_total_size += stats_file.size;
        }
    }
error:
    OS_Free(dir_hd);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    OS_Free(stats_file.long_name_p);
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    return s;
}

/******************************************************************************/
Status OS_FileCreate(OS_FileHd*fhd_p, ConstStrPtr file_path_p, const OS_FileOpenMode op_mode)
{
OS_FileOpenMode op_mode_create = op_mode;
    OS_LOG(D_DEBUG, "File create: %s", file_path_p);
    BIT_SET(op_mode_create, BIT(OS_FS_FILE_OP_MODE_CREATE_NEW));
    return OS_FileOpen(fhd_p, file_path_p, op_mode_create);
}

/******************************************************************************/
Status OS_FileDelete(ConstStrPtr file_path_p)
{
    OS_LOG(D_DEBUG, "File delete: %s", file_path_p);
    return FResultTranslate(f_unlink((const char*)file_path_p));
}

/******************************************************************************/
Status OS_FileOpen(OS_FileHd*fhd_p, ConstStrPtr file_path_p, const OS_FileOpenMode op_mode)
{
*fhd_p = OS_Malloc(sizeof(FIL));
const OS_FileHd fhd = *fhd_p;
Status s;
    if (OS_NULL == fhd) { return S_NO_MEMORY; }
    s = FResultTranslate(f_open(fhd, (const char*)file_path_p, FOpenModeConvert(op_mode)));
    OS_LOG(D_DEBUG, "File open : 0x%X %s", fhd, file_path_p);
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

/******************************************************************************/
Status OS_FileClose(OS_FileHd* fhd_p)
{
const OS_FileHd fhd = *fhd_p;
    OS_LOG(D_DEBUG, "File close: 0x%X", fhd);
Status s = FResultTranslate(f_close(fhd));
    OS_Free(fhd);
    *fhd_p = OS_NULL;
    return s;
}

/******************************************************************************/
Status OS_FileRead(const OS_FileHd fhd, U8* data_in_p, U32 size)
{
UINT bytes_read;
    OS_LOG(D_DEBUG, "File read: 0x%X", fhd);
Status s = FResultTranslate(f_read(fhd, data_in_p, size, &bytes_read));
    if (bytes_read != size) {
        s = S_SIZE_MISMATCH;
    }
    return s;
}

/******************************************************************************/
Status OS_FileWrite(const OS_FileHd fhd, U8* data_out_p, U32 size)
{
UINT bytes_written;
    OS_LOG(D_DEBUG, "File write: 0x%X", fhd);
Status s = FResultTranslate(f_write(fhd, data_out_p, size, &bytes_written));
    if (bytes_written != size) {
        s = S_SIZE_MISMATCH;
    }
    return s;
}

/******************************************************************************/
Status OS_FileRename(ConstStrPtr name_old_p, ConstStrPtr name_new_p)
{
    OS_LOG(D_DEBUG, "File rename: %s -> %s", name_old_p, name_new_p);
    return FResultTranslate(f_rename((const char*)name_old_p, (const char*)name_new_p));
}

/******************************************************************************/
Status OS_FileDateTimeSet(ConstStrPtr file_path_p, OS_DateTime date_time)
{
FILINFO file_info;
    date_time.year -= OS_FILE_SYSTEM_YEAR_BASE;
const DWORD fattime = FDateTimeConvert(date_time);
    file_info.fdate = (fattime >> 16) & 0xFFFF;
    file_info.ftime = fattime & 0xFFFF;
    OS_LOG(D_DEBUG, "File date/time set: %s", file_path_p);
    return FResultTranslate(f_utime((const char*)file_path_p, &file_info));
}

/******************************************************************************/
Status OS_FileAttributesSet(ConstStrPtr file_path_p, const OS_FileAttrs attrs)
{
    OS_LOG(D_DEBUG, "File attrs set: %s", file_path_p);
    return FResultTranslate(f_chmod((const char*)file_path_p, FAttributesConvert(attrs), (BYTE)~0U));
}

/******************************************************************************/
Status OS_FileGetS(const OS_FileHd fhd, StrPtr str_p, U32 len)
{
    if (str_p != (StrPtr)f_gets((char*)str_p, len, fhd)) {
        if (f_eof(fhd)) {
            return S_FS_EOF;
        } else if (f_error(fhd)) {
            return S_FS_UNDEF;
        } else {
            return S_FS_UNDEF;
        }
    }
    return S_OK;
}

/******************************************************************************/
Status OS_FilePutS(const OS_FileHd fhd, StrPtr str_p)
{
    if (0 > f_puts((char*)str_p, fhd)) {
        return S_FS_UNDEF;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_FilePutC(const OS_FileHd fhd, U8 chr)
{
    if (0 > f_putc((char)chr, fhd)) {
        return S_FS_UNDEF;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_FileLSeek(const OS_FileHd fhd, const U32 offset)
{
    return FResultTranslate(f_lseek(fhd, (DWORD)offset));
}

/******************************************************************************/
U32 OS_FileTell(const OS_FileHd fhd)
{
    //if (OS_FILE_UNDEF == fhd) { return U32_MAX; }
    return (f_tell(fhd));
}

/******************************************************************************/
Status OS_DirectoryCreate(ConstStrPtr path_p)
{
    OS_LOG(D_DEBUG, "Dir create: %s", path_p);
    return FResultTranslate(f_mkdir((const char*)path_p));
}

/******************************************************************************/
Status OS_DirectoryDelete(ConstStrPtr path_p)
{
    OS_LOG(D_DEBUG, "Dir delete: %s", path_p);
    return OS_FileDelete(path_p);
}

/******************************************************************************/
Status OS_DirectoryOpen(OS_DirHd dhd, StrPtr path_p)
{
Status s = FResultTranslate(f_opendir(dhd, (const char*)path_p));
    //FDirTranslate(&dir, dhd_p);
    OS_LOG(D_DEBUG, "Dir open: 0x%X %s", dhd, path_p);
    return s;
}

/******************************************************************************/
Status OS_DirectoryRead(OS_DirHd dhd, OS_FileStats* file_stats_p)
{
FILINFO file_info;
    OS_LOG(D_DEBUG, "Dir read: 0x%X", dhd);
    if ((OS_NULL == dhd) || (OS_NULL == file_stats_p)) { return S_INVALID_REF; }
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
    file_info.lfname = (char*)file_stats_p->long_name_p;
    file_info.lfsize = file_stats_p->long_name_size;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    Status s = FResultTranslate(f_readdir(dhd, &file_info));
    IF_STATUS_OK(s) {
        OS_MemCpy8(file_stats_p->name, file_info.fname, sizeof(file_stats_p->name));
        file_stats_p->size          = file_info.fsize;
        file_stats_p->date_time     = FDateTimeTranslate(file_info.fdate, file_info.ftime);
        file_stats_p->attrs         = FAttributeTranslate(file_info.fattrib);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
        file_stats_p->long_name_p   = (StrPtr)file_info.lfname;
        file_stats_p->long_name_size= file_info.lfsize;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    }
    return s;
}

/******************************************************************************/
Status OS_DirectoryRename(ConstStrPtr name_old_p, ConstStrPtr name_new_p)
{
    OS_LOG(D_DEBUG, "Dir rename: %s -> %s", name_old_p, name_new_p);
    return OS_FileRename(name_old_p, name_new_p);
}

/******************************************************************************/
int MMC_disk_initialize(OS_DriverHd* drv_sdio_sd_p)
{
SD_CardInfo card_info;
DSTATUS dstatus = 0;

    *drv_sdio_sd_p = drv_media_v[MMC];
    if (SD_Detect() != SD_PRESENT) {
        BIT_SET(dstatus, STA_NODISK);
    } else {
        if (SD_OK == SD_GetCardInfo(&card_info)) {
            if (card_info.SD_csd.PermWrProtect) {
                BIT_SET(dstatus, STA_PROTECT);
            }
        } else {
            BIT_SET(dstatus, STA_NOINIT);
        }
    }
    return dstatus;
}

/******************************************************************************/
int MMC_disk_status(void)
{
DSTATUS dstatus = 0;
    if (SD_Detect() != SD_PRESENT) {
        BIT_SET(dstatus, STA_NODISK);
    }
    return dstatus;
}

/******************************************************************************/
DWORD get_fattime(void);
DWORD get_fattime(void)
{
OS_DateTime date_time;
    OS_TimeGet(OS_TIME_UNDEF, &date_time);
    OS_DateGet(OS_DATE_UNDEF, &date_time);
    date_time.year -= OS_FILE_SYSTEM_YEAR_BASE;
    return FDateTimeConvert(date_time);
}

/******************************************************************************/
OS_DateTime FDateTimeTranslate(const WORD date, const WORD time)
{
OS_DateTime time_date;
    time_date.year  = BF_GET(date, 9,  BIT_MASK(7)) + OS_FILE_SYSTEM_YEAR_BASE;
    time_date.month = BF_GET(date, 5,  BIT_MASK(4));
    time_date.day   = BF_GET(date, 0,  BIT_MASK(4));

    time_date.hour  = BF_GET(time, 11, BIT_MASK(5));
    time_date.min   = BF_GET(time, 5,  BIT_MASK(6));
    time_date.sec   = BF_GET(time, 0,  BIT_MASK(4));
    return time_date;
}

/******************************************************************************/
DWORD FDateTimeConvert(const OS_DateTime date_time)
{
DWORD fattime;
    //TODO(A. Filyanov) Use BF macro.
    fattime  = (((U32)date_time.year    & 0x7F) << 25);
    fattime |= (((U32)date_time.month   & 0x0F) << 21);
    fattime |= (((U32)date_time.day     & 0x1F) << 16);

    fattime |= (((U32)date_time.hour    & 0x1F) << 11);
    fattime |= (((U32)date_time.min     & 0x3F) << 5);
    fattime |= (date_time.sec           & 0x1F);
    return fattime;
}

/******************************************************************************/
BYTE FOpenModeConvert(const OS_FileOpenMode op_mode)
{
BYTE fmode = 0;
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_CREATE_NEW))) {
        fmode |= FA_CREATE_NEW;
    }
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_CREATE_EXISTS))) {
        fmode |= FA_CREATE_ALWAYS;
    }
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_OPEN_NEW))) {
        fmode |= FA_OPEN_ALWAYS;
    }
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_OPEN_EXISTS))) {
        fmode |= FA_OPEN_EXISTING;
    }
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_READ))) {
        fmode |= FA_READ;
    }
    if (BIT_TEST(op_mode, (OS_FileOpenMode)BIT(OS_FS_FILE_OP_MODE_WRITE))) {
        fmode |= FA_WRITE;
    }
    return fmode;
}

/******************************************************************************/
OS_FileAttrs FAttributeTranslate(const BYTE a)
{
OS_FileAttrs attrs = OS_FS_FILE_ATTR_UNDEF;
    if (BIT_TEST(a, AM_VOL)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_VOL));
    }
    if (BIT_TEST(a, AM_DIR)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_DIR));
    }
    if (BIT_TEST(a, AM_RDO)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_RDO));
    }
    if (BIT_TEST(a, AM_HID)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_HID));
    }
    if (BIT_TEST(a, AM_SYS)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_SYS));
    }
    if (BIT_TEST(a, AM_ARC)) {
        BIT_SET(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_ARC));
    }
    return attrs;
}

/******************************************************************************/
BYTE FAttributesConvert(const OS_FileAttrs attrs)
{
BYTE fs_attrs = 0;
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_VOL))) {
        fs_attrs |= OS_FS_FILE_ATTR_VOL;
    }
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_DIR))) {
        fs_attrs |= AM_DIR;
    }
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_RDO))) {
        fs_attrs |= AM_RDO;
    }
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_HID))) {
        fs_attrs |= AM_HID;
    }
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_SYS))) {
        fs_attrs |= AM_SYS;
    }
    if (BIT_TEST(attrs, (OS_FileAttrs)BIT(OS_FS_FILE_ATTR_ARC))) {
        fs_attrs |= AM_ARC;
    }
    return fs_attrs;
}

/******************************************************************************/
Status FResultTranslate(const FRESULT r)
{
Status s = S_OK;
    switch (r) {
        case FR_OK:
            break;
        case FR_DISK_ERR:
            s = S_FS_MEDIA_FAULT;
            break;
        case FR_INT_ERR:
            s = S_FS_ASSERT;
            break;
        case FR_NOT_READY:
            s = S_FS_NOT_READY;
            break;
        case FR_NO_FILE:
            s = S_FS_FILE_NOT_FOUND;
            break;
        case FR_NO_PATH:
            s = S_FS_PATH_NOT_FOUND;
            break;
        case FR_INVALID_NAME:
            s = S_FS_FILENAME_INVALID;
            break;
        case FR_DENIED:
            s = S_FS_ACCESS_DENIED;
            break;
        case FR_EXIST:
            s = S_FS_OBJECT_EXISTS;
            break;
        case FR_INVALID_OBJECT:
            s = S_FS_OBJECT_INVALID;
            break;
        case FR_WRITE_PROTECTED:
            s = S_FS_WRITE_PROTECTED;
            break;
        case FR_INVALID_DRIVE:
            s = S_FS_MEDIA_INVALID;
            break;
        case FR_NOT_ENABLED:
            s = S_FS_NOT_ENABLED;
            break;
        case FR_NO_FILESYSTEM:
            s = S_FS_NO_FILESYSTEM;
            break;
        case FR_MKFS_ABORTED:
            s = S_FS_MKFS_ABORTED;
            break;
        case FR_TIMEOUT:
            s = S_FS_TIMEOUT;
            break;
        case FR_LOCKED:
            s = S_FS_LOCKED;
            break;
        case FR_NOT_ENOUGH_CORE:
            s = S_FS_ALLOCATION;
            break;
        case FR_TOO_MANY_OPEN_FILES:
            s = S_FS_TOO_MANY_OPENED_FILES;
            break;
        case FR_INVALID_PARAMETER:
            s = S_FS_INVALID_PARAMETER;
            break;
        default:
            OS_ASSERT(OS_FALSE);
            break;
    }
    return s;
}

/******************************************************************************/
Status VolumeCheck(const S8 volume)
{
    if ((0 > volume) || (OS_FILE_SYSTEM_VOLUMES_MAX < volume)) { return S_FS_MEDIA_INVALID; }
    return S_OK;
}

#endif // (1 == OS_FILE_SYSTEM_ENABLED)