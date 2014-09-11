/**************************************************************************//**
* @file    drv_usbh_msc.c
* @brief   USB Host MSC driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#if (1 == USBH_ENABLED)
#if (1 == USBH_MSC_ENABLED)
#include "usbh_msc.h"
#include "diskio.h"
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_message.h"
#include "os_signal.h"
#include "os_debug.h"
#include "os_file_system.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usb_msc"

//------------------------------------------------------------------------------
#if (1 == USBH_FS_ENABLED)
static Status   USBH_FS_MSC_Init(void* args_p);
static Status   USBH_FS_MSC_DeInit(void* args_p);
static Status   USBH_FS_MSC_Open(void* args_p);
static Status   USBH_FS_MSC_Close(void* args_p);
static Status   USBH_FS_MSC_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_FS_MSC_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_FS_MSC_IoCtl(const U32 request_id, void* args_p);
#endif //(1 == USBH_FS_ENABLED)
#if (1 == USBH_HS_ENABLED)
static Status   USBH_HS_MSC_Init(void* args_p);
static Status   USBH_HS_MSC_DeInit(void* args_p);
static Status   USBH_HS_MSC_Open(void* args_p);
static Status   USBH_HS_MSC_Close(void* args_p);
static Status   USBH_HS_MSC_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_HS_MSC_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_HS_MSC_IoCtl(const U32 request_id, void* args_p);
#endif //(1 == USBH_HS_ENABLED)

//------------------------------------------------------------------------------
#if (1 == USBH_FS_ENABLED)
static USBH_HandleTypeDef* usbh_fs_hd_p;
static U8 usbh_fs_msc_lun = 0; //!!!Driver (by now) support only logical unit 0!!!
#endif //(1 == USBH_FS_ENABLED)

#if (1 == USBH_HS_ENABLED)
static USBH_HandleTypeDef* usbh_hs_hd_p;
static U8 usbh_hs_msc_lun = 0; //!!!Driver (by now) support only logical unit 0!!!
#endif //(1 == USBH_HS_ENABLED)

#if (1 == USBH_FS_ENABLED)
/*static*/ HAL_DriverItf drv_usbh_fs_msc = {
    .Init   = USBH_FS_MSC_Init,
    .DeInit = USBH_FS_MSC_DeInit,
    .Open   = USBH_FS_MSC_Open,
    .Close  = USBH_FS_MSC_Close,
    .Read   = USBH_FS_MSC_Read,
    .Write  = USBH_FS_MSC_Write,
    .IoCtl  = USBH_FS_MSC_IoCtl
};
#endif //(1 == USBH_FS_ENABLED)

#if (1 == USBH_HS_ENABLED)
/*static*/ HAL_DriverItf drv_usbh_hs_msc = {
    .Init   = USBH_HS_MSC_Init,
    .DeInit = USBH_HS_MSC_DeInit,
    .Open   = USBH_HS_MSC_Open,
    .Close  = USBH_HS_MSC_Close,
    .Read   = USBH_HS_MSC_Read,
    .Write  = USBH_HS_MSC_Write,
    .IoCtl  = USBH_HS_MSC_IoCtl
};
#endif //(1 == USBH_HS_ENABLED)

#if (1 == USBH_FS_ENABLED)
/******************************************************************************/
Status USBH_FS_MSC_Init(void* args_p)
{
Status s = S_OK;
    if (OS_NULL != args_p) {
        usbh_fs_hd_p = (USBH_HandleTypeDef*)args_p;
    }
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
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Close(void* args_p)
{
Status s = S_OK;
    s = drv_usbh_fs_msc.IoCtl(DRV_REQ_STD_SYNC, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Read(U8* data_in_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
//State state = ON;
Status s = S_OK;
    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            IF_STATUS(s = drv_usbh_fs_msc.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MemCpy(data_in_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            data_in_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

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
//    state = OFF;
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_Write(U8* data_out_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
//State state = ON;
Status s = S_OK;
    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            OS_MemCpy(data_out_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            IF_STATUS(s = drv_usbh_fs_msc.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

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
//    state = OFF;
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_FS_MSC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
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
            if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
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
                if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            break;
        case DRV_REQ_FS_MEDIA_STATUS_GET:
            if(0 == USBH_MSC_UnitIsReady(usbh_fs_hd_p, usbh_fs_msc_lun)) {
                s = S_FS_NOT_READY;
            }
            break;
        case GET_SECTOR_COUNT: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_nbr;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case GET_SECTOR_SIZE:
        case GET_BLOCK_SIZE: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_fs_hd_p, usbh_fs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_size;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_ERASE_SECTOR:
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}
#endif //(1 == USBH_FS_ENABLED)

#if (1 == USBH_HS_ENABLED)
/******************************************************************************/
Status USBH_HS_MSC_Init(void* args_p)
{
Status s = S_OK;
    if (OS_NULL != args_p) {
        usbh_hs_hd_p = (USBH_HandleTypeDef*)args_p;
    }
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
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Close(void* args_p)
{
Status s = S_OK;
    s = drv_usbh_hs_msc.IoCtl(DRV_REQ_STD_SYNC, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Read(U8* data_in_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
//State state = ON;
Status s = S_OK;
    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            IF_STATUS(s = drv_usbh_hs_msc.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MemCpy(data_in_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            data_in_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

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
//    state = OFF;
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_Write(U8* data_out_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
//State state = ON;
Status s = S_OK;
    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(USBH_MSC_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            OS_MemCpy(data_out_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            IF_STATUS(s = drv_usbh_hs_msc.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

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
//    state = OFF;
//    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_HS_MSC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
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
            if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
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
                if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            break;
        case DRV_REQ_FS_MEDIA_STATUS_GET:
            if(0 == USBH_MSC_UnitIsReady(usbh_hs_hd_p, usbh_hs_msc_lun)) {
                s = S_FS_NOT_READY;
            }
            break;
        case GET_SECTOR_COUNT: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_nbr;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case GET_SECTOR_SIZE:
        case GET_BLOCK_SIZE: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hs_hd_p, usbh_hs_msc_lun, &info)) {
                *(U32*)args_p = info.capacity.block_size;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_ERASE_SECTOR:
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}
#endif //(1 == USBH_HS_ENABLED)

#endif //(1 == USBH_MSC_ENABLED)
#endif //(1 == USBH_ENABLED)
