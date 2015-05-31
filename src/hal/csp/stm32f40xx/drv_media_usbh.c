/**************************************************************************//**
* @file    drv_media_usbh.c
* @brief   USB Host media driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#if (HAL_USBH_ENABLED)
#if (HAL_USBH_MSC_ENABLED)
#include "usbh_msc.h"
#include "diskio.h"
#include "drv_media_usbh.h"
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_mailbox.h"
#include "os_signal.h"
#include "os_debug.h"
#include "os_file_system.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_m_usbh"

//------------------------------------------------------------------------------
#if defined(OS_MEDIA_VOL_USBH_FS) && (USBH_FS_ENABLED)
static Status   USBH_FS_MSC_Init(void* args_p);
static Status   USBH_FS_MSC_DeInit(void* args_p);
static Status   USBH_FS_MSC_Open(void* args_p);
static Status   USBH_FS_MSC_Close(void* args_p);
static Status   USBH_FS_MSC_Read(void* data_in_p, Size size, void* args_p);
static Status   USBH_FS_MSC_Write(void* data_out_p, Size size, void* args_p);
static Status   USBH_FS_MSC_IoCtl(const U32 request_id, void* args_p);
#endif //defined(OS_MEDIA_VOL_USBH_FS) && (USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
static Status   USBH_HS_MSC_Init(void* args_p);
static Status   USBH_HS_MSC_DeInit(void* args_p);
static Status   USBH_HS_MSC_Open(void* args_p);
static Status   USBH_HS_MSC_Close(void* args_p);
static Status   USBH_HS_MSC_Read(void* data_in_p, Size size, void* args_p);
static Status   USBH_HS_MSC_Write(void* data_out_p, Size size, void* args_p);
static Status   USBH_HS_MSC_IoCtl(const U32 request_id, void* args_p);
#endif //(HAL_USBH_HS_ENABLED)

//------------------------------------------------------------------------------
#if (HAL_USBH_FS_ENABLED)
static USBH_HandleTypeDef* usbh_fs_hd_p;
static U8 usbh_fs_msc_lun = 0; //!!!Driver (currently) supports only logical unit 0!!!
#endif //(HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
static USBH_HandleTypeDef* usbh_hs_hd_p;
static U8 usbh_hs_msc_lun = 0; //!!!Driver (currently) supports only logical unit 0!!!
#endif //(HAL_USBH_HS_ENABLED)

#if defined(OS_MEDIA_VOL_USBH_FS) && (HAL_USBH_FS_ENABLED)
/*static*/ HAL_DriverItf drv_media_usbh_fs = {
    .Init   = USBH_FS_MSC_Init,
    .DeInit = USBH_FS_MSC_DeInit,
    .Open   = USBH_FS_MSC_Open,
    .Close  = USBH_FS_MSC_Close,
    .Read   = USBH_FS_MSC_Read,
    .Write  = USBH_FS_MSC_Write,
    .IoCtl  = USBH_FS_MSC_IoCtl
};
#endif //defined(OS_MEDIA_VOL_USBH_FS) && (HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
/*static*/ HAL_DriverItf drv_media_usbh_hs = {
    .Init   = USBH_HS_MSC_Init,
    .DeInit = USBH_HS_MSC_DeInit,
    .Open   = USBH_HS_MSC_Open,
    .Close  = USBH_HS_MSC_Close,
    .Read   = USBH_HS_MSC_Read,
    .Write  = USBH_HS_MSC_Write,
    .IoCtl  = USBH_HS_MSC_IoCtl
};
#endif //(HAL_USBH_HS_ENABLED)

static OS_DriverHd drv_led_fs;

#if defined(OS_MEDIA_VOL_USBH_FS) && (HAL_USBH_FS_ENABLED)
/******************************************************************************/
Status USBH_FS_MSC_Init(void* args_p)
{
const DrvMediaUsbArgsInit* drv_args_p = (DrvMediaUsbArgsInit*)args_p;
Status s = S_OK;
    usbh_fs_hd_p    = (USBH_HandleTypeDef*)drv_args_p->usb_itf_hd;
    drv_led_fs      = drv_args_p->drv_led_fs;
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_DeInit(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Open(void* args_p)
{
Status s = S_OK;
    IF_STATUS(s = OS_DriverOpen(drv_led_fs, OS_NULL)) {}
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Close(void* args_p)
{
Status s = S_OK;
    IF_OK(s = drv_media_usbh_fs.IoCtl(DRV_REQ_STD_SYNC, OS_NULL)) {
        IF_OK(s = OS_DriverClose(drv_led_fs, OS_NULL)) {}
    }
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Read(void* data_in_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
//    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
//        U32* scratch_p = OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
//        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
//        while (size--) {
//            U8* data_in_8p = data_in_p;
//            IF_STATUS(s = drv_media_usbh_fs.Read((U8*)scratch_p, 1, &sector)) { break; }
//            OS_MemCpy(data_in_p, scratch_p, USBH_MSC_BLOCK_SIZE);
//            data_in_8p += USBH_MSC_BLOCK_SIZE;
//            ++sector;
//        }
//        OS_Free(scratch_p);
//        return s;
//    }

    const USBH_StatusTypeDef usbh_status = USBH_MSC_Read(usbh_fs_hd_p, usbh_fs_msc_lun, sector, data_in_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info);

        switch (info.sense.asc) {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
              s = S_FS_NOT_READY;
              break;
            default:
              s = S_FS_TRANSFER_FAIL;
              break;
        }
    }
    state = OFF;
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Write(void* data_out_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
//    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
//        U32* scratch_p = OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
//        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
//        while (size--) {
//            U8* data_out_8p = data_out_p;
//            OS_MemCpy(data_out_p, scratch_p, USBH_MSC_BLOCK_SIZE);
//            IF_STATUS(s = drv_media_usbh_fs.Write((U8*)scratch_p, 1, &sector)) { break; }
//            data_out_8p += USBH_MSC_BLOCK_SIZE;
//            ++sector;
//        }
//        OS_Free(scratch_p);
//        return s;
//    }

    const USBH_StatusTypeDef usbh_status = USBH_MSC_Write(usbh_fs_hd_p, usbh_fs_msc_lun, sector, data_out_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info);

        switch (info.sense.asc) {
            case SCSI_ASC_WRITE_PROTECTED:
              s = S_FS_WRITE_PROTECTED;
              break;
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
              s = S_FS_NOT_READY;
              break;
            default:
              s = S_FS_TRANSFER_FAIL;
              break;
        }
    }
    state = OFF;
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_LOG(D_DEBUG, "ioctl id=%d", request_id);
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
//                    if (SD_POWER_STATE_OFF == USBH_GetPowerState(sd_handle.Instance)) {
//                        hal_status = USBH_PowerState_ON(sd_handle.Instance);
//                    }
                    break;
                case PWR_OFF: {
//                    if (SD_POWER_STATE_ON == USBH_GetPowerState(sd_handle.Instance)) {
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
//                    } else if (SD_POWER_STATE_UP == USBH_GetPowerState(sd_handle.Instance)) {
//                        while (SD_POWER_STATE_ON != USBH_GetPowerState(sd_handle.Instance)) {};
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
//                    }
                    }
                    break;
                default:
                    break;
            }
            if (HAL_OK == hal_status) {
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_POWER: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            //Power Get
            //....
            //Power Set
            const U8 power_state = (*(U8*)args_p) & 0xFF;
                switch (power_state) {
                    case PWR_ON:
//                        hal_status = USBH_PowerState_ON(sd_handle.Instance);
                        break;
                    case PWR_OFF:
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
                        break;
                    default:
                        break;
                }
                if (HAL_OK == hal_status) {
                    s = S_OK;
                } else {
                    s = S_FS_UNDEF;
                }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_STATUS_GET:
            if (0 == USBH_MSC_UnitIsReady(usbh_fs_hd_p, usbh_fs_msc_lun)) {
                s = S_FS_NOT_READY;
            } else {
                s = S_OK;
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_COUNT_GET:
        case GET_SECTOR_COUNT: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_nbr;
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_SIZE_GET:
        case GET_SECTOR_SIZE:
        case DRV_REQ_MEDIA_BLOCK_SIZE_GET:
        case GET_BLOCK_SIZE: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info)) {
                *(U16*)args_p = info.capacity.block_size;
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_ERASE_SECTOR:
            s = S_OK;
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}
#endif //defined(OS_MEDIA_VOL_USBH_FS) && (HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
/******************************************************************************/
Status USBH_HS_MSC_Init(void* args_p)
{
const DrvMediaUsbArgsInit* drv_args_p = (DrvMediaUsbArgsInit*)args_p;
Status s = S_OK;
    usbh_hs_hd_p    = (USBH_HandleTypeDef*)drv_args_p->usb_itf_hd;
    drv_led_fs      = drv_args_p->drv_led_fs;
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_DeInit(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Open(void* args_p)
{
Status s = S_OK;
    IF_STATUS(s = OS_DriverOpen(drv_led_fs, OS_NULL)) {}
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Close(void* args_p)
{
Status s = S_OK;
    IF_OK(s = drv_media_usbh_hs.IoCtl(DRV_REQ_STD_SYNC, OS_NULL)) {
        IF_OK(s = OS_DriverClose(drv_led_fs, OS_NULL)) {}
    }
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Read(void* data_in_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
//    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
//        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_Size); // Alignment assured
//        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
//        while (size--) {
//            IF_STATUS(s = drv_media_usbh_hs.Read((U8*)scratch_p, 1, &sector)) { break; }
//            OS_MemCpy(data_in_p, scratch_p, USBH_MSC_BLOCK_Size);
//            data_in_p += USBH_MSC_BLOCK_Size;
//            ++sector;
//        }
//        OS_Free(scratch_p);
//        return s;
//    }

    const USBH_StatusTypeDef usbh_status = USBH_MSC_Read(usbh_hs_hd_p, usbh_hs_msc_lun, sector, data_in_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info);

        switch (info.sense.asc) {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
              s = S_FS_NOT_READY;
              break;
            default:
              s = S_FS_TRANSFER_FAIL;
              break;
        }
    }
    state = OFF;
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Write(void* data_out_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
//    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
//        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_Size); // Alignment assured
//        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
//        while (size--) {
//            OS_MemCpy(data_out_p, scratch_p, USBH_MSC_BLOCK_Size);
//            IF_STATUS(s = drv_media_usbh_hs.Write((U8*)scratch_p, 1, &sector)) { break; }
//            data_out_p += USBH_MSC_BLOCK_Size;
//            ++sector;
//        }
//        OS_Free(scratch_p);
//        return s;
//    }

    const USBH_StatusTypeDef usbh_status = USBH_MSC_Write(usbh_hs_hd_p, usbh_hs_msc_lun, sector, data_out_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info);

        switch (info.sense.asc) {
            case SCSI_ASC_WRITE_PROTECTED:
              s = S_FS_WRITE_PROTECTED;
              break;
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
              s = S_FS_NOT_READY;
              break;
            default:
              s = S_FS_TRANSFER_FAIL;
              break;
        }
    }
    state = OFF;
//    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_LOG(D_DEBUG, "ioctl req_id=%d", request_id);
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
//                    if (SD_POWER_STATE_OFF == USBH_GetPowerState(sd_handle.Instance)) {
//                        hal_status = USBH_PowerState_ON(sd_handle.Instance);
//                    }
                    break;
                case PWR_OFF: {
//                    if (SD_POWER_STATE_ON == USBH_GetPowerState(sd_handle.Instance)) {
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
//                    } else if (SD_POWER_STATE_UP == USBH_GetPowerState(sd_handle.Instance)) {
//                        while (SD_POWER_STATE_ON != USBH_GetPowerState(sd_handle.Instance)) {};
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
//                    }
                    }
                    break;
                default:
                    break;
            }
            if (HAL_OK == hal_status) {
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_POWER: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            //Power Get
            //....
            //Power Set
            const U8 power_state = (*(U8*)args_p) & 0xFF;
                switch (power_state) {
                    case PWR_ON:
//                        hal_status = USBH_PowerState_ON(sd_handle.Instance);
                        break;
                    case PWR_OFF:
//                        hal_status = USBH_PowerState_OFF(sd_handle.Instance);
                        break;
                    default:
                        break;
                }
                if (HAL_OK == hal_status) {
                    s = S_OK;
                } else {
                    s = S_FS_UNDEF;
                }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_STATUS_GET:
            if(0 == USBH_MSC_UnitIsReady(usbh_hs_hd_p, usbh_hs_msc_lun)) {
                s = S_FS_NOT_READY;
            } else {
                s = S_OK;
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_COUNT_GET:
        case GET_SECTOR_COUNT: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_nbr;
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_Size_GET:
        case GET_SECTOR_Size:
        case DRV_REQ_MEDIA_BLOCK_Size_GET:
        case GET_BLOCK_Size: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info)) {
                *(U16*)args_p = info.capacity.block_size;
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_ERASE_SECTOR:
            s = S_OK;
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}
#endif //(HAL_USBH_HS_ENABLED)

#endif //(HAL_USBH_MSC_ENABLED)
#endif //(HAL_USBH_ENABLED)
