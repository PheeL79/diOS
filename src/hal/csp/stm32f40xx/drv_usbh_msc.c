/**************************************************************************//**
* @file    drv_usbh_msc.c
* @brief   USB Host MSC driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
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
#define MDL_NAME            "drv_usbh_msc"

//------------------------------------------------------------------------------
static Status   USBH_MSC_Init(void* args_p);
static Status   USBH_MSC_DeInit(void);
static Status   USBH_MSC_Open(void* args_p);
static Status   USBH_MSC_Close(void);
static Status   USBH_MSC_Read_(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_MSC_Write_(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_MSC_IT_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_MSC_IT_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_MSC_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_MSC_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_MSC_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
static USBH_HandleTypeDef*  usbh_hd_p;
static const U8             usbh_fs_lun = 0;

/*static*/ HAL_DriverItf drv_usbh_msc = {
    .Init   = USBH_MSC_Init,
    .DeInit = USBH_MSC_DeInit,
    .Open   = USBH_MSC_Open,
    .Close  = USBH_MSC_Close,
    .Read   = USBH_MSC_Read_,
    .Write  = USBH_MSC_Write_,
    .IoCtl  = USBH_MSC_IoCtl
};

/******************************************************************************/
Status USBH_MSC_Init(void* args_p)
{
Status s = S_OK;
    if (OS_NULL != args_p) {
        usbh_hd_p = (USBH_HandleTypeDef*)args_p;
    }
    return s;
}

/******************************************************************************/
Status USBH_MSC_DeInit(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_Open(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_Close(void)
{
Status s = S_OK;
    s = drv_usbh_msc.IoCtl(DRV_REQ_STD_SYNC, OS_NULL);
    return s;
}

/******************************************************************************/
Status USBH_MSC_Read_(U8* data_in_p, U32 size, void* args_p)
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
            IF_STATUS(s = drv_usbh_msc.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MEMCPY(data_in_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            data_in_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    const USBH_StatusTypeDef usbh_status =
        USBH_MSC_Read(usbh_hd_p, usbh_fs_lun, sector, data_in_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_hd_p, usbh_fs_lun, &info);

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
Status USBH_MSC_Write_(U8* data_out_p, U32 size, void* args_p)
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
            OS_MEMCPY(data_out_p, scratch_p, USBH_MSC_BLOCK_SIZE);
            IF_STATUS(s = drv_usbh_msc.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_p += USBH_MSC_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    const USBH_StatusTypeDef usbh_status =
        USBH_MSC_Write(usbh_hd_p, usbh_fs_lun, sector, data_out_p, size);
    if (USBH_OK != usbh_status) {
        MSC_LUNTypeDef info;
        USBH_MSC_GetLUNInfo(usbh_hd_p, usbh_fs_lun, &info);

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
Status USBH_MSC_IT_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_IT_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_MSC_IoCtl(const U32 request_id, void* args_p)
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
            if(0 == USBH_MSC_UnitIsReady(usbh_hd_p, usbh_fs_lun)) {
                s = S_FS_NOT_READY;
            }
            break;
        case GET_SECTOR_COUNT: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hd_p, usbh_fs_lun, &info)) {
                *(U32*)args_p = info.capacity.block_nbr;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case GET_SECTOR_SIZE:
        case GET_BLOCK_SIZE: {
            MSC_LUNTypeDef info;
            if (USBH_OK == USBH_MSC_GetLUNInfo(usbh_hd_p, usbh_fs_lun, &info)) {
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
