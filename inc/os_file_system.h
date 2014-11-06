/***************************************************************************//**
* @file    os_file_system.h
* @brief   OS File system.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_FILE_SYSTEM_H_
#define _OS_FILE_SYSTEM_H_

#include "ff.h"
#include "os_common.h"
#include "os_driver.h"
#include "os_time.h"

#if (1 == OS_FILE_SYSTEM_ENABLED)

/**
* \defgroup OS_FileSystem OS_FileSystem
* @{
*/
//------------------------------------------------------------------------------
typedef void*   OS_FileSystemMediaHd;
typedef FATFS*  OS_FileSystemHd;
typedef FIL*    OS_FileHd;
typedef DIR*    OS_DirHd;

typedef enum {
    S_FS_UNDEF = S_MODULE,
    S_FS_UNMOUNTED,
    S_FS_ISNT_INITED,
    S_FS_MEDIA_FAULT,
    S_FS_ASSERT,
    S_FS_NOT_READY,
    S_FS_FILE_NOT_FOUND,
    S_FS_PATH_NOT_FOUND,
    S_FS_FILENAME_INVALID,
    S_FS_ACCESS_DENIED,
    S_FS_OBJECT_EXISTS,
    S_FS_OBJECT_INVALID,
    S_FS_WRITE_PROTECTED,
    S_FS_MEDIA_INVALID,
    S_FS_NOT_ENABLED,
    S_FS_NO_FILESYSTEM,
    S_FS_MKFS_ABORTED,
    S_FS_TIMEOUT,
    S_FS_LOCKED,
    S_FS_ALLOCATION,
    S_FS_TOO_MANY_OPENED_FILES,
    S_FS_INVALID_PARAMETER,
    S_FS_TRANSFER_FAIL,
    S_FS_EOF
} OS_FileSystemStatus;

typedef enum {
    OS_FS_UNDEF,
    OS_FS_FAT12,
    OS_FS_FAT16,
    OS_FS_FAT32,
    OS_FS_LAST
} OS_FileSystemType;

typedef enum {
    OS_FS_PART_RULE_UNDEF,
    OS_FS_PART_RULE_FDISK,
    OS_FS_PART_RULE_SFD,
    OS_FS_PART_RULE_LAST
} OS_FileSystemPartitionRule;

typedef struct {
    Str             name[OS_FILE_SYSTEM_VOLUME_NAME_LEN];
    OS_DriverConfig*drv_cfg_p;
    U8              volume;
} OS_FileSystemMediaConfig;

typedef struct {
    StrPtr              media_name_p;
    Str                 name[OS_FILE_SYSTEM_VOLUME_NAME_LEN];
    U32                 serial;
    OS_FileSystemType   type;
    U32                 clusters_count;
    U32                 cluster_size;
    U32                 clusters_free;
    U32                 root_dir_items_count;
    U32                 fats_count;
    U32                 fats_sectors;
    U32                 base_vol;
    U32                 base_fat;
    U32                 base_dir;
    U32                 base_data;
} OS_FileSystemVolumeStats;

typedef struct {
    U32                 files_count;
    U32                 files_total_size;
    U32                 dirs_count;
} OS_FileSystemStats;

enum {
    OS_FS_FILE_ATTR_UNDEF,
    OS_FS_FILE_ATTR_VOL,
    OS_FS_FILE_ATTR_DIR,
    OS_FS_FILE_ATTR_RDO,
    OS_FS_FILE_ATTR_HID,
    OS_FS_FILE_ATTR_SYS,
    OS_FS_FILE_ATTR_ARC,
    OS_FS_FILE_ATTR_LFN,
    OS_FS_FILE_ATTR_LAST
};
typedef U8 OS_FileAttrs;

enum {
    OS_FS_FILE_OP_MODE_UNDEF,
    OS_FS_FILE_OP_MODE_CREATE_NEW,
    OS_FS_FILE_OP_MODE_CREATE_EXISTS,
    OS_FS_FILE_OP_MODE_OPEN_NEW,
    OS_FS_FILE_OP_MODE_OPEN_EXISTS,
    OS_FS_FILE_OP_MODE_READ,
    OS_FS_FILE_OP_MODE_WRITE,
    OS_FS_FILE_OP_MODE_LAST
};
typedef U8 OS_FileOpenMode;

typedef struct {
    U32             size;
    OS_DateTime     date_time;
    OS_FileAttrs    attrs;
    Str             name[13];
#if OS_FILE_SYSTEM_LONG_NAMES_ENABLED
    StrPtr          long_name_p;
    U32             long_name_size;
#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
} OS_FileStats;

//typedef struct {
//    OS_FileSystemHd fshd;
//    U16             id;
//    U16             rw_idx;
//    U32             cluster_start;
//    U32             cluster_curr;
//    U32             sector_curr;
//    StrPtr          dir_name_p;
//    StrPtr          name_ext_stat_p;
//#if OS_FILE_SYSTEM_LONG_NAMES_ENABLED
//    StrPtr          long_name_p;
//    U16             long_name_idx;
//#endif // OS_FILE_SYSTEM_LONG_NAMES_ENABLED
//} OS_DirStats;

//------------------------------------------------------------------------------
/// @brief      Init the file system.
/// @return     #Status.
Status          OS_FileSystemInit(void);

/// @brief      Deinit the file system.
/// @return     #Status.
Status          OS_FileSystemDeInit(void);

/// @brief      Create the file system media volume.
/// @param[in]  cfg_p           Media config.
/// @param[out] fs_media_hd_p   Media handle.
/// @return     #Status.
Status          OS_FileSystemMediaCreate(const OS_FileSystemMediaConfig* cfg_p, OS_FileSystemMediaHd* fs_media_hd_p);

/// @brief      Delete the file system media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @return     #Status.
Status          OS_FileSystemMediaDelete(const OS_FileSystemMediaHd fs_media_hd);

/// @brief      Init the file system media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @param[in]  args_p          Media driver init arguments.
/// @return     #Status.
Status          OS_FileSystemMediaInit(const OS_FileSystemMediaHd fs_media_hd, void* args_p);

/// @brief      Deinit the file system media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @return     #Status.
Status          OS_FileSystemMediaDeInit(const OS_FileSystemMediaHd fs_media_hd);

//S8              OS_FileSystemMediaCurrentGet(void);

/// @brief      Set the system current media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @return     #Status.
Status          OS_FileSystemMediaCurrentSet(const OS_FileSystemMediaHd fs_media_hd);

/// @brief      Get the media current status.
/// @param[in]  fs_media_hd     Media handle.
/// @return     #Status.
Status          OS_FileSystemMediaStatusGet(const OS_FileSystemMediaHd fs_media_hd);

/// @brief      Get the media volume name.
/// @param[in]  fs_media_hd     Media handle.
/// @return     Volume name.
ConstStrPtr     OS_FileSystemMediaNameGet(const OS_FileSystemMediaHd fs_media_hd);

/// @brief      Make file system on media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @param[in]  part_rule       Make partition rule.
/// @param[in]  size            Partition size.
/// @return     #Status.
Status          OS_FileSystemMake(const OS_FileSystemMediaHd fs_media_hd, const OS_FileSystemPartitionRule part_rule, const SIZE size);

/// @brief      Mount file system on media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @param[in]  args_p          Media driver open arguments.
/// @return     #Status.
Status          OS_FileSystemMount(const OS_FileSystemMediaHd fs_media_hd, void* args_p);

/// @brief      Unmount file system on media volume.
/// @param[in]  fs_media_hd     Media handle.
/// @return     #Status.
Status          OS_FileSystemUnMount(const OS_FileSystemMediaHd fs_media_hd);

OS_FileSystemMediaHd OS_FileSystemMediaByVolumeGet(const U8 volume);

/// @brief      Get media handle by it's volume name.
/// @param[in]  name_p          Media volume name.
/// @return     Media handle.
OS_FileSystemMediaHd OS_FileSystemMediaByNameGet(ConstStrPtr name_p);

U8              OS_FileSystemVolumeGet(const OS_FileSystemMediaHd fs_media_hd);

/// @brief      Get media volume label.
/// @param[in]  fs_media_hd     Media handle.
/// @param[out] label_p         Media volume label.
/// @param[out] serial_p        Media volume serial.
/// @return     #Status.
Status          OS_FileSystemVolumeLabelGet(const OS_FileSystemMediaHd fs_media_hd, StrPtr label_p, U32* serial_p);

/// @brief      Set media volume label.
/// @param[in]  fs_media_hd     Media handle.
/// @param[in]  label_p         Media volume label.
/// @return     #Status.
Status          OS_FileSystemVolumeLabelSet(const OS_FileSystemMediaHd fs_media_hd, StrPtr label_p);

/// @brief      Get media volume statistics.
/// @param[in]  fs_media_hd     Media handle.
/// @param[out] stats_p         Media volume statistics.
/// @return     #Status.
Status          OS_FileSystemVolumeStatsGet(const OS_FileSystemMediaHd fs_media_hd, OS_FileSystemVolumeStats* stats_p);

/// @brief      Scan volume for statistics.
/// @param[in]  path_p          Volume path.
/// @param[out] stats_p         File system statistics.
/// @return     #Status.
Status          OS_FileSystemVolumeScan(const StrPtr path_p, OS_FileSystemStats* stats_p);

/// @brief      Get media volume free clusters.
/// @param[in]  fs_media_hd     Media handle.
/// @param[in]  path_p          Volume path.
/// @param[out] clusters_free_count_p Free clusters count.
/// @return     #Status.
Status          OS_FileSystemClustersFreeGet(const OS_FileSystemMediaHd fs_media_hd, const StrPtr path_p, U32* clusters_free_count_p);

/**
* \addtogroup OS_FileSystemFilesOps File system files operations.
* @{
*/
/// @brief      Create a file.
/// @param[out] fhd_p           File handle.
/// @param[in]  file_path_p     File path.
/// @param[in]  op_mode         File open mode.
/// @return     #Status.
Status          OS_FileCreate(OS_FileHd* fhd_p, ConstStrPtr file_path_p, const OS_FileOpenMode op_mode);

/// @brief      Delete file.
/// @param[in]  file_path_p     File path.
/// @return     #Status.
Status          OS_FileDelete(ConstStrPtr file_path_p);

/// @brief      Open file.
/// @param[out] fhd_p           File handle.
/// @param[in]  file_path_p     File path.
/// @param[in]  op_mode         File open mode.
/// @return     #Status.
Status          OS_FileOpen(OS_FileHd* fhd_p, ConstStrPtr file_path_p, const OS_FileOpenMode op_mode);

/// @brief      Close file.
/// @param[in]  fhd_p           File handle.
/// @return     #Status.
Status          OS_FileClose(OS_FileHd* fhd_p);

/// @brief      Read file.
/// @param[in]  fhd             File handle.
/// @param[out] data_in_p       Data input buffer.
/// @param[in]  size            Input buffer size.
/// @return     #Status.
Status          OS_FileRead(const OS_FileHd fhd, U8* data_in_p, U32 size);

/// @brief      Write file.
/// @param[in]  fhd             File handle.
/// @param[in]  data_out_p      Data output buffer.
/// @param[in]  size            Output buffer size.
/// @return     #Status.
Status          OS_FileWrite(const OS_FileHd fhd, U8* data_out_p, U32 size);

/// @brief      Rename file.
/// @param[in]  name_old_p      Old name path.
/// @param[in]  name_new_p      New name path.
/// @return     #Status.
Status          OS_FileRename(ConstStrPtr name_old_p, ConstStrPtr name_new_p);

/// @brief      Set file date and time.
/// @param[in]  file_path_p     File path.
/// @param[in]  date_time       Date and time.
/// @return     #Status.
Status          OS_FileDateTimeSet(ConstStrPtr file_path_p, OS_DateTime date_time);

/// @brief      Set file attributes.
/// @param[in]  file_path_p     File path.
/// @param[in]  attrs           File attributes.
/// @return     #Status.
Status          OS_FileAttributesSet(ConstStrPtr file_path_p, const OS_FileAttrs attrs);

/// @brief      Get file string.
/// @param[in]  fhd             File handle.
/// @param[out] str_p           String.
/// @param[in]  len             String length.
/// @return     #Status.
Status          OS_FileGetS(const OS_FileHd fhd, StrPtr str_p, U32 len);

/// @brief      Put string to the file.
/// @param[in]  fhd             File handle.
/// @param[in]  str_p           String.
/// @return     #Status.
Status          OS_FilePutS(const OS_FileHd fhd, StrPtr str_p);

/// @brief      Put char to the file.
/// @param[in]  fhd             File handle.
/// @param[in]  chr             Char.
/// @return     #Status.
Status          OS_FilePutC(const OS_FileHd fhd, U8 chr);

/// @brief      Set file read/write offset.
/// @param[in]  fhd             File handle.
/// @param[in]  offset          Offset (bytes).
/// @return     #Status.
Status          OS_FileLSeek(const OS_FileHd fhd, const U32 offset);

/// @brief      Get file read/write offset.
/// @param[in]  fhd             File handle.
/// @return     Offset (bytes).
U32             OS_FileTell(const OS_FileHd fhd);
/**@}*/ //OS_FileSystemFilesOps

/**
* \addtogroup OS_FileSystemDirectoriesOps File system directories operations.
* @{
*/
/// @brief      Create a directory.
/// @param[in]  path_p          Directory path.
/// @return     #Status.
Status OS_DirectoryCreate(ConstStrPtr path_p);

/// @brief      Delete directory.
/// @param[in]  path_p          Directory path.
/// @return     #Status.
Status OS_DirectoryDelete(ConstStrPtr path_p);

/// @brief      Open directory.
/// @param[in]  dhd             Directory handle.
/// @param[in]  path_p          Directory path.
/// @return     #Status.
Status OS_DirectoryOpen(OS_DirHd dhd, StrPtr path_p);

/// @brief      Read directory item.
/// @param[in]  dhd             Directory handle.
/// @param[in]  file_stats_p    Directory file statistics.
/// @return     #Status.
Status OS_DirectoryRead(OS_DirHd dhd, OS_FileStats* file_stats_p);

/// @brief      Rename directory.
/// @param[in]  name_old_p      Old name path.
/// @param[in]  name_new_p      New name path.
/// @return     #Status.
Status OS_DirectoryRename(ConstStrPtr name_old_p, ConstStrPtr name_new_p);

/// @brief      Set directory date and time.
/// @param[in]  dir_path_p      Directory path.
/// @param[in]  date_time       Date and time.
/// @return     #Status.
Status OS_DirectoryDateTimeSet(ConstStrPtr dir_path_p, const OS_DateTime date_time);
/**@}*/ //OS_FileSystemDirectoriesOps

/**@}*/ //OS_FileSystem

#endif // (1 == OS_FILE_SYSTEM_ENABLED)

#endif // _OS_FILE_SYSTEM_H_
