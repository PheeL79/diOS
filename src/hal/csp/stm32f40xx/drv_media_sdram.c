/**************************************************************************//**
* @file    drv_media_sdram.c
* @brief   SDRAM media driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "diskio.h"
#include "os_common.h"
#include "os_debug.h"
#include "os_driver.h"
#include "os_memory.h"
#include "os_file_system.h"

#if defined(OS_MEDIA_VOL_SDRAM)
//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_m_sdram"

//-----------------------------------------------------------------------------
static Status SDRAM_Init_(void* args_p);
static Status SDRAM_DeInit_(void* args_p);
//static Status SDRAM_LL_Init(void* args_p);
//static Status SDRAM_LL_DeInit(void* args_p);
static Status SDRAM_Open(void* args_p);
static Status SDRAM_Close(void* args_p);
static Status SDRAM_Read(void* data_in_p, Size size, void* args_p);
static Status SDRAM_Write(void* data_out_p, Size size, void* args_p);
//static Status SDRAM_DMA_Read(void* data_in_p, Size size, void* args_p);
//static Status SDRAM_DMA_Write(void* data_out_p, Size size, void* args_p);
static Status SDRAM_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
extern SRAM_HandleTypeDef sram1_hd;
static OS_DriverHd drv_led_fs;
static void* sdram_fs_p;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_media_sdram = {
    .Init   = SDRAM_Init_,
    .DeInit = SDRAM_DeInit_,
    .Open   = SDRAM_Open,
    .Close  = SDRAM_Close,
    .Read   = SDRAM_Read,
    .Write  = SDRAM_Write,
    .IoCtl  = SDRAM_IoCtl
};

/*****************************************************************************/
Status SDRAM_Init_(void* args_p)
{
Status s = S_OK;
    HAL_LOG(D_INFO, "Init: ");
    drv_led_fs = *(OS_DriverHd*)args_p;
    sdram_fs_p = OS_MallocEx(OS_MEDIA_VOL_SDRAM_SIZE, OS_MEDIA_VOL_SDRAM_MEM);
    if (OS_NULL == sdram_fs_p) { s = S_NO_MEMORY; }
    return s;
}

/*****************************************************************************/
Status SDRAM_DeInit_(void* args_p)
{
Status s = S_OK;
    OS_FreeEx(sdram_fs_p, OS_MEDIA_VOL_SDRAM_MEM);
    return s;
}

/*****************************************************************************/
Status SDRAM_Open(void* args_p)
{
Status s = S_OK;
    IF_STATUS(s = OS_DriverOpen(drv_led_fs, OS_NULL)) {}
    return s;
}

/*****************************************************************************/
Status SDRAM_Close(void* args_p)
{
Status s = S_OK;
    IF_STATUS_OK(s = drv_media_sdram.IoCtl(DRV_REQ_STD_SYNC, OS_NULL)) {
        IF_STATUS_OK(s = OS_DriverClose(drv_led_fs, OS_NULL)) {}
    }
    return s;
}

/******************************************************************************/
Status SDRAM_Read(void* data_in_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State led_fs_state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (HAL_OK != HAL_SRAM_Read_8b(&sram1_hd,
//                                   (uint32_t *)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_Size * sector)),
//                                   data_in_p,
//                                   (OS_MEDIA_VOL_SDRAM_BLOCK_Size * size))) {
//        s = S_HARDWARE_FAULT;
//    }
    OS_MemCpy(data_in_p, (const void*)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * sector)), (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * size));
    led_fs_state = OFF;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDRAM_Write(void* data_out_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
State led_fs_state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (HAL_OK != HAL_SRAM_Write_8b(&sram1_hd,
//                                    (uint32_t *)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_Size * sector)),
//                                    data_out_p,
//                                    (OS_MEDIA_VOL_SDRAM_BLOCK_Size * size))) {
//        s = S_HARDWARE_FAULT;
//    }
    OS_MemCpy((void*)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * sector)), data_out_p, (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * size));
    led_fs_state = OFF;
    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
//Status SDRAM_DMA_Read(void* data_in_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//State led_fs_state = ON;
//Status s = S_OK;
////    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (HAL_OK != HAL_SRAM_Read_DMA(&sram1_hd,
//                                    (uint32_t *)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * sector)),
//                                    (uint32_t *)data_in_p,
//                                    (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * size))) {
//        s = S_HARDWARE_FAULT;
//    }
//    led_fs_state = OFF;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    return s;
//}

/******************************************************************************/
//Status SDRAM_DMA_Write(void* data_out_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//State led_fs_state = ON;
//Status s = S_OK;
////    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (HAL_OK != HAL_SRAM_Write_DMA(&sram1_hd,
//                                     (uint32_t *)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * sector)),
//                                     (uint32_t *)data_out_p,
//                                     (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * size))) {
//        s = S_HARDWARE_FAULT;
//    }
//    led_fs_state = OFF;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    return s;
//}

/******************************************************************************/
Status SDRAM_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_LOG(D_DEBUG, "ioctl req_id=%d", request_id);
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                    s = S_OK;
                    break;
                default:
                    break;
            }
            }
            break;
        case CTRL_POWER:
            s = S_OK;
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            OS_MemCacheFlush();
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_STATUS_GET:
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_SECTOR_COUNT_GET:
        case GET_SECTOR_COUNT:
            *(U32*)args_p = OS_MEDIA_VOL_SDRAM_SIZE / OS_MEDIA_VOL_SDRAM_BLOCK_SIZE;
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_SECTOR_SIZE_GET:
        case GET_SECTOR_SIZE:
            *(U16*)args_p = OS_MEDIA_VOL_SDRAM_BLOCK_SIZE;
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_BLOCK_SIZE_GET:
        case GET_BLOCK_SIZE:
            *(U16*)args_p = OS_MEDIA_VOL_SDRAM_BLOCK_SIZE;
            s = S_OK;
            break;
        case CTRL_ERASE_SECTOR: {
            U32 start_sector= ((U32*)args_p)[0] * OS_MEDIA_VOL_SDRAM_BLOCK_SIZE;
            U32 end_sector  = ((U32*)args_p)[1] * OS_MEDIA_VOL_SDRAM_BLOCK_SIZE;
            OS_MemSet((void*)((U8*)sdram_fs_p + (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * start_sector)),
                      0,
                      (OS_MEDIA_VOL_SDRAM_BLOCK_SIZE * (end_sector - start_sector)));
            }
            s = S_OK;
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}

// SDRAM IRQ handlers -----------------------------------------------------------

#endif //defined(OS_MEDIA_VOL_SDRAM)