/***************************************************************************//**
* @file    os_file_system.c
* @brief   OS File system.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "os_debug.h"
#include "os_memory.h"
#include "os_driver.h"
#include "os_list.h"
#include "os_time.h"
#include "os_file_system.h"

#if (1 == OS_FILE_SYSTEM_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "file_system"
#undef  MDL_STATUS_ITEMS
#define MDL_STATUS_ITEMS    &status_fs_v[0]

//------------------------------------------------------------------------------
typedef struct {
    Str             name[OS_FILE_SYSTEM_VOLUME_NAME_LEN];
    Str             volume[OS_FILE_SYSTEM_VOLUME_STR_LEN];
    OS_DriverHd     dhd;
    OS_FileSystemHd fshd;
} OS_FileSystemMediaConfigDyn;

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
static OS_List os_fs_list;
static OS_MutexHd os_fs_mutex;
OS_DriverHd fs_media_dhd_v[OS_FILE_SYSTEM_VOLUMES_MAX];

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
#pragma inline
static OS_FileSystemMediaConfigDyn* OS_FileSystemMediaConfigDynGet(const OS_FileSystemMediaHd fs_media_hd);
OS_FileSystemMediaConfigDyn* OS_FileSystemMediaConfigDynGet(const OS_FileSystemMediaHd fs_media_hd)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    const OS_ListItem* item_l_p = (OS_ListItem*)fs_media_hd;
    OS_FileSystemMediaConfigDyn* cfg_dyn_p = (OS_FileSystemMediaConfigDyn*)OS_ListItemValueGet(item_l_p);
    OS_ASSERT_VALUE(OS_DELAY_MAX != (OS_Value)cfg_dyn_p);
    return cfg_dyn_p;
}

/******************************************************************************/
Status OS_FileSystemInit(void)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init");
    os_fs_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_fs_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_fs_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_fs_list)) { return S_INVALID_VALUE; }
    OS_MemSet(fs_media_dhd_v, 0, sizeof(fs_media_dhd_v));
s = S_OK;
    return s;
}

/******************************************************************************/
Status OS_FileSystemDeInit(void)
{
Status s = S_UNDEF;
    //TODO(A. Filyanov) Loop and delete over existed media handles.
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaCreate(const OS_FileSystemMediaConfig* cfg_p, OS_FileSystemMediaHd* fs_media_hd_p)
{
Status s = S_UNDEF;
    if (OS_NULL == cfg_p) { return S_INVALID_REF; }
    IF_STATUS(s = VolumeCheck(cfg_p->volume)) { return s; }
    if (OS_NULL != OS_FileSystemMediaByVolumeGet(cfg_p->volume)) { return S_FS_MEDIA_INVALID; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_FileSystemMediaConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    cfg_dyn_p->fshd = OS_Malloc(sizeof(FATFS));
    if (OS_NULL == cfg_dyn_p->fshd) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    IF_STATUS(s = OS_DriverCreate(cfg_p->drv_cfg_p, &cfg_dyn_p->dhd)) {
        OS_Free(cfg_dyn_p->fshd);
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        return s;
    }
    fs_media_dhd_v[cfg_p->volume] = cfg_dyn_p->dhd;
    cfg_dyn_p->volume[0]= cfg_p->volume + '0';
    cfg_dyn_p->volume[1]= OS_FILE_SYSTEM_DRV_DELIM;
    cfg_dyn_p->volume[2]= OS_FILE_SYSTEM_DIR_DELIM;
    cfg_dyn_p->volume[3]= '\0'; //EOL
    OS_StrNCpy(cfg_dyn_p->name, (const char*)cfg_p->name, sizeof(cfg_dyn_p->name));
    OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
    OS_ListItemOwnerSet(item_l_p, (OS_Owner)cfg_p->volume);
    if (OS_NULL != fs_media_hd_p) {
        *fs_media_hd_p = (OS_FileSystemMediaHd)item_l_p;
    }
    IF_OK(s = OS_MutexRecursiveLock(os_fs_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListAppend(&os_fs_list, item_l_p);
        OS_MutexRecursiveUnlock(os_fs_mutex);
    }
//error:
    IF_STATUS(s) {
        Status s_drv;
        IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
        OS_Free(cfg_dyn_p->fshd);
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        fs_media_dhd_v[cfg_p->volume] = OS_NULL;
    }
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaDelete(const OS_FileSystemMediaHd fs_media_hd)
{
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_fs_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* item_l_p = (OS_ListItem*)fs_media_hd;
        OS_FileSystemMediaConfigDyn* cfg_dyn_p = (OS_FileSystemMediaConfigDyn*)OS_ListItemValueGet(item_l_p);
        const U8 volume = (U8)OS_ListItemOwnerGet(item_l_p);
        fs_media_dhd_v[volume] = OS_NULL;
        s = OS_DriverDelete(cfg_dyn_p->dhd);
        OS_Free(cfg_dyn_p->fshd);
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        OS_MutexRecursiveUnlock(os_fs_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaInit(const OS_FileSystemMediaHd fs_media_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS media init: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    {
        const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
        const OS_DriverHd drv_media_hd = cfg_dyn_p->dhd;
        OS_DriverStats stats;
        IF_STATUS(s = OS_DriverStatsGet(drv_media_hd, &stats)) { return s; }
        if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_INIT))) {
            IF_STATUS(s = OS_DriverInit(drv_media_hd, args_p)) { return s; }
        }
    }
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaDeInit(const OS_FileSystemMediaHd fs_media_hd)
{
Status s;
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS media deinit: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    const OS_DriverHd drv_media_hd = cfg_dyn_p->dhd;
    IF_STATUS(s = OS_DriverIoCtl(drv_media_hd, DRV_REQ_STD_SYNC, OS_NULL)) {
        OS_LOG_S(D_WARNING, s);
    }
    if (OS_NULL != drv_media_hd) {
        OS_FileSystemUnMount(fs_media_hd);
    }
    IF_STATUS(s = OS_DriverDeInit(drv_media_hd, OS_NULL)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

/******************************************************************************/
//S8 OS_FileSystemMediaCurrentGet(void)
//{
//    return volume_curr;
//}

/******************************************************************************/
OS_FileSystemMediaHd OS_FileSystemMediaByVolumeGet(const U8 volume)
{
OS_FileSystemMediaHd fs_media_hd = OS_NULL;
    IF_OK(OS_MutexRecursiveLock(os_fs_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        fs_media_hd = (OS_FileSystemMediaHd)OS_ListItemByOwnerGet(&os_fs_list, (OS_Owner)volume);
        OS_MutexRecursiveUnlock(os_fs_mutex);
    }
    return fs_media_hd;
}

/******************************************************************************/
Status OS_FileSystemMediaCurrentSet(const OS_FileSystemMediaHd fs_media_hd)
{
Status s;
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS media set: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    IF_OK(s = FResultTranslate(f_chdrive((const char*)cfg_dyn_p->volume))) {
//        volume_curr = volume;
    }
    return s;
}

/******************************************************************************/
Status OS_FileSystemMediaStatusGet(const OS_FileSystemMediaHd fs_media_hd)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return OS_DriverIoCtl(cfg_dyn_p->dhd, DRV_REQ_MEDIA_STATUS_GET, OS_NULL);
}

/******************************************************************************/
ConstStrP OS_FileSystemMediaNameGet(const OS_FileSystemMediaHd fs_media_hd)
{
    if (OS_NULL == fs_media_hd) { return OS_NULL; }
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return cfg_dyn_p->name;
}

/******************************************************************************/
OS_DriverHd OS_FileSystemMediaDriverGet(const OS_FileSystemMediaHd fs_media_hd);
OS_DriverHd OS_FileSystemMediaDriverGet(const OS_FileSystemMediaHd fs_media_hd)
{
    if (OS_NULL == fs_media_hd) { return OS_NULL; }
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return cfg_dyn_p->dhd;
}

#if (1 == OS_FILE_SYSTEM_MAKE_ENABLED)
/******************************************************************************/
Status OS_FileSystemMake(const OS_FileSystemMediaHd fs_media_hd, const OS_FileSystemPartitionRule part_rule, const Size size)
{
const BYTE fpart_rule = (OS_FS_PART_RULE_FDISK == part_rule) ? 0 :
                        (OS_FS_PART_RULE_SFD   == part_rule) ? 1 : U8_MAX;
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS make: %s, rule: %d, size: %u", OS_FileSystemMediaNameGet(fs_media_hd), part_rule, size);
    if (U8_MAX == fpart_rule) { return S_FS_INVALID_PARAMETER; }
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return FResultTranslate(f_mkfs((const char*)cfg_dyn_p->volume, fpart_rule, size));
}
#endif // (1 == OS_FILE_SYSTEM_MAKE_ENABLED)

/******************************************************************************/
Status OS_FileSystemMount(const OS_FileSystemMediaHd fs_media_hd, void* args_p)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS mount volume: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
#ifndef NDEBUG
    OS_MemSet(cfg_dyn_p->fshd, 0xF5, sizeof(FATFS));
#endif // NDEBUG
    OS_DriverStats stats;
    Status s;
    IF_STATUS(s = OS_DriverStatsGet(cfg_dyn_p->dhd, &stats)) { return s; }
    if (!BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverOpen(cfg_dyn_p->dhd, args_p)) { return s; }
    }
    return FResultTranslate(f_mount(cfg_dyn_p->fshd, (const char*)cfg_dyn_p->volume, 1));
}

/******************************************************************************/
Status OS_FileSystemUnMount(const OS_FileSystemMediaHd fs_media_hd)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS unmount volume: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    OS_DriverStats stats;
    Status s;
    IF_STATUS(s = FResultTranslate(f_mount(OS_NULL, (const char*)cfg_dyn_p->volume, 1))) { return s; }
    IF_STATUS(s = OS_DriverStatsGet(cfg_dyn_p->dhd, &stats)) { return s; }
    if (BIT_TEST(stats.state, BIT(OS_DRV_STATE_IS_OPEN))) {
        IF_STATUS(s = OS_DriverClose(cfg_dyn_p->dhd, OS_NULL)) { return s; }
    }
    return s;
}

/******************************************************************************/
U8 OS_FileSystemVolumeGet(const OS_FileSystemMediaHd fs_media_hd)
{
    if (OS_NULL == fs_media_hd) { return U8_MAX; }
    OS_ListItem* item_l_p = (OS_ListItem*)fs_media_hd;
    return (U8)OS_ListItemOwnerGet(item_l_p);
}

/******************************************************************************/
//S8 OS_FileSystemVolumeByNameGet(ConstStrP name_p)
//{
//    for (S8 i = 0; i < OS_FILE_SYSTEM_VOLUMES_MAX; ++i) {
//        if (!OS_STRCMP((const char*)name_p, (const char*)media_label_v[i].name_p)) {
//            return i;
//        }
//    }
//    return -1;
//}

/******************************************************************************/
Status OS_FileSystemVolumeLabelGet(const OS_FileSystemMediaHd fs_media_hd, StrP label_p, U32* serial_p)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS vol label get: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return FResultTranslate(f_getlabel((const char*)cfg_dyn_p->volume,
                                       (char*)label_p,
                                       (DWORD*)serial_p));
}

/******************************************************************************/
Status OS_FileSystemVolumeLabelSet(const OS_FileSystemMediaHd fs_media_hd, StrP label_p)
{
Str vol_label[OS_FILE_SYSTEM_VOLUME_STR_LEN + OS_FILE_SYSTEM_VOLUME_NAME_LEN];
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS vol label set: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    OS_StrNCpy(vol_label, (char const*)cfg_dyn_p->volume, OS_FILE_SYSTEM_VOLUME_STR_LEN);
    OS_StrCat( vol_label, (char const*)label_p);
    return FResultTranslate(f_setlabel((const char*)vol_label));
}

/******************************************************************************/
Status OS_FileSystemVolumeStatsGet(const OS_FileSystemMediaHd fs_media_hd, OS_FileSystemVolumeStats* stats_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    if (OS_NULL == stats_p) { return S_INVALID_REF; }
    OS_LOG(D_DEBUG, "FS vol stats get: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    FATFS* fs_p = cfg_dyn_p->fshd;
    OS_MemSet(stats_p, 0x0, sizeof(OS_FileSystemVolumeStats));
    {
    DWORD clusters_count;
        IF_STATUS(s = FResultTranslate(f_chdrive((const char*)cfg_dyn_p->volume))) { return s; }
        IF_STATUS(s = FResultTranslate(f_getfree((const char*)cfg_dyn_p->volume,
                                                &clusters_count, &fs_p))) { return s; }
        IF_STATUS(s = OS_FileSystemVolumeLabelGet(fs_media_hd,
                                                 (StrP)stats_p->name,
                                                 &stats_p->serial)) { return s; }
    }
    stats_p->media_name_p           = (StrP)cfg_dyn_p->name;
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
Status OS_FileSystemClustersFreeGet(const OS_FileSystemMediaHd fs_media_hd, const StrP path_p, U32* clusters_free_count_p)
{
    OS_ASSERT_VALUE(OS_NULL != fs_media_hd);
    OS_LOG(D_DEBUG, "FS clusters free: %s", OS_FileSystemMediaNameGet(fs_media_hd));
    const OS_FileSystemMediaConfigDyn* cfg_dyn_p = OS_FileSystemMediaConfigDynGet(fs_media_hd);
    return FResultTranslate(f_getfree((const char*)path_p, (DWORD*)clusters_free_count_p, (FATFS**)&cfg_dyn_p->fshd));
}

/******************************************************************************/
Status OS_FileSystemVolumeScan(const StrP path_p, OS_FileSystemStats* stats_p)
{
OS_DirHd dir_hd;
OS_FileStats stats_file;
Size path_len;
StrP file_name_p;
Status s = S_UNDEF;
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
    path_len = OS_StrLen((const char*)path_p);
    while ((S_OK == (s = OS_DirectoryRead(dir_hd, &stats_file))) && ('\0' != stats_file.name[0])) {
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
        file_name_p = (*stats_file.long_name_p) ? stats_file.long_name_p : stats_file.name;
#else
        file_name_p = stats_file.name;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
        if (BIT_TEST(stats_file.attrs, BIT(OS_FS_FILE_ATTR_DIR))) {
            // Guard relative directories.
            if (OS_NULL != OS_StrChr((const char*)file_name_p, '.')) { continue; }
            stats_p->dirs_count++;
            *(path_p + path_len) = OS_FILE_SYSTEM_DIR_DELIM;
            OS_StrCpy((char*)(path_p + path_len + 1), (const char*)file_name_p);
            s = OS_FileSystemVolumeScan(path_p, stats_p);
            *(path_p + path_len) = '\0'; //EOL;
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
Status OS_FileCreate(OS_FileHd*fhd_p, ConstStrP file_path_p, const OS_FileOpenMode op_mode)
{
OS_FileOpenMode op_mode_create = op_mode;
    OS_LOG(D_DEBUG, "File create: %s", file_path_p);
    BIT_SET(op_mode_create, BIT(OS_FS_FILE_OP_MODE_CREATE_NEW));
    return OS_FileOpen(fhd_p, file_path_p, op_mode_create);
}

/******************************************************************************/
Status OS_FileDelete(ConstStrP file_path_p)
{
    OS_LOG(D_DEBUG, "File delete: %s", file_path_p);
    return FResultTranslate(f_unlink((const char*)file_path_p));
}

/******************************************************************************/
Status OS_FileOpen(OS_FileHd*fhd_p, ConstStrP file_path_p, const OS_FileOpenMode op_mode)
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
Status OS_FileRead(const OS_FileHd fhd, void* data_in_p, Size size)
{
UInt bytes_read;
    OS_LOG(D_DEBUG, "File read: 0x%X", fhd);
Status s = FResultTranslate(f_read(fhd, data_in_p, size, &bytes_read));
    if (0 == bytes_read) {
        s = S_FS_EOF;
    } else if (size != bytes_read) {
        s = S_SIZE_MISMATCH;
    }
    return s;
}

/******************************************************************************/
Status OS_FileWrite(const OS_FileHd fhd, void* data_out_p, Size size)
{
UInt bytes_written;
    OS_LOG(D_DEBUG, "File write: 0x%X", fhd);
Status s = FResultTranslate(f_write(fhd, data_out_p, size, &bytes_written));
    if (bytes_written != size) {
        s = S_SIZE_MISMATCH;
    }
    return s;
}

/******************************************************************************/
Status OS_FileRename(ConstStrP name_old_p, ConstStrP name_new_p)
{
    OS_LOG(D_DEBUG, "File rename: %s -> %s", name_old_p, name_new_p);
    return FResultTranslate(f_rename((const char*)name_old_p, (const char*)name_new_p));
}

/******************************************************************************/
Status OS_FileDateTimeSet(ConstStrP file_path_p, OS_DateTime date_time)
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
Status OS_FileAttributesSet(ConstStrP file_path_p, const OS_FileAttrs attrs)
{
    OS_LOG(D_DEBUG, "File attrs set: %s", file_path_p);
    return FResultTranslate(f_chmod((const char*)file_path_p, FAttributesConvert(attrs), (BYTE)~0U));
}

/******************************************************************************/
Status OS_FileGetS(const OS_FileHd fhd, StrP str_p, U32 len)
{
    if (str_p != (StrP)f_gets((char*)str_p, len, fhd)) {
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
Status OS_FilePutS(const OS_FileHd fhd, StrP str_p)
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
Status OS_DirectoryCreate(ConstStrP path_p)
{
    OS_LOG(D_DEBUG, "Dir create: %s", path_p);
    return FResultTranslate(f_mkdir((const char*)path_p));
}

/******************************************************************************/
Status OS_DirectoryDelete(ConstStrP path_p)
{
    OS_LOG(D_DEBUG, "Dir delete: %s", path_p);
    return OS_FileDelete(path_p);
}

/******************************************************************************/
Status OS_DirectoryOpen(OS_DirHd dhd, StrP path_p)
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
    IF_OK(s) {
        OS_MemCpy(file_stats_p->name, file_info.fname, sizeof(file_stats_p->name));
        file_stats_p->size          = file_info.fsize;
        file_stats_p->date_time     = FDateTimeTranslate(file_info.fdate, file_info.ftime);
        file_stats_p->attrs         = FAttributeTranslate(file_info.fattrib);
#if defined(OS_FILE_SYSTEM_LONG_NAMES_ENABLED)
        file_stats_p->long_name_p   = (StrP)file_info.lfname;
        file_stats_p->long_name_size= file_info.lfsize;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    }
    return s;
}

/******************************************************************************/
Status OS_DirectoryRename(ConstStrP name_old_p, ConstStrP name_new_p)
{
    OS_LOG(D_DEBUG, "Dir rename: %s -> %s", name_old_p, name_new_p);
    return OS_FileRename(name_old_p, name_new_p);
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
    time_date.year      = BF_GET(date, 9,  7) + OS_FILE_SYSTEM_YEAR_BASE;
    time_date.month     = BF_GET(date, 5,  4);
    time_date.day       = BF_GET(date, 0,  4);

    time_date.hours     = BF_GET(time, 11, 5);
    time_date.minutes   = BF_GET(time, 5,  6);
    time_date.seconds   = BF_GET(time, 0,  4);
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

    fattime |= (((U32)date_time.hours   & 0x1F) << 11);
    fattime |= (((U32)date_time.minutes & 0x3F) << 5);
    fattime |= (date_time.seconds       & 0x1F);
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
Status s = S_UNDEF;
    switch (r) {
        case FR_OK:
            s = S_OK;
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