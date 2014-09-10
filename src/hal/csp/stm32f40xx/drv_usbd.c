/**************************************************************************//**
* @file    drv_usbd.c
* @brief   USB Device driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_message.h"
#include "os_signal.h"
#include "os_debug.h"
#if (1 == USBD_ENABLED)
#include "usbd_core.h"
#if (1 == USBD_HID_ENABLED)
    #include "usbd_hid.h"
#endif // USBD_HID_ENABLED
#if (1 == USBD_MSC_ENABLED)
    #include "usbd_msc.h"
#endif // USBD_MSC_ENABLED

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usbd"

//------------------------------------------------------------------------------
/// @brief   USBD initialization.
/// @return  #Status.
Status USBD__Init(void);

static Status   USBD_Init_(void* args_p);
static Status   USBD_DeInit_(void* args_p);
static Status   USBD_Open(void* args_p);
static Status   USBD_Close(void* args_p);
static Status   USBD_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
#if (1 == USBD_FS_ENABLED)
static HCD_HandleTypeDef    hcd_fs_hd;
static USBD_HandleTypeDef*  usbd_fs_hd_p;
#endif //(1 == USBD_FS_ENABLED)
#if (1 == USBD_HS_ENABLED)
static HCD_HandleTypeDef    hcd_hs_hd;
static USBD_HandleTypeDef*  usbd_hs_hd_p;
#endif //(1 == USBD_HS_ENABLED)
//OS_QueueHd                  usbdd_stdin_qhd;
HAL_DriverItf*              drv_usbd_v[DRV_ID_USBD_LAST];

static HAL_DriverItf drv_usbd = {
    .Init   = USBD_Init_,
    .DeInit = USBD_DeInit_,
    .Open   = USBD_Open,
    .Close  = USBD_Close,
    .IoCtl  = USBD_IoCtl
};

/******************************************************************************/
Status USBD__Init(void)
{
Status s = S_OK;
    HAL_MEMSET(drv_usbd_v, 0x0, sizeof(drv_usbd_v));
    drv_usbd_v[DRV_ID_USBD] = &drv_usbd;
#if (1 == USBD_MSC_ENABLED)
    extern HAL_DriverItf drv_usbd_fs_msc, drv_usbd_hs_msc;
#if (1 == USBD_FS_ENABLED)
    drv_usbd_v[DRV_ID_USBD_FS_MSC] = &drv_usbd_fs_msc;
#endif //(1 == USBD_FS_ENABLED)
#if (1 == USBD_HS_ENABLED)
    drv_usbd_v[DRV_ID_USBD_HS_MSC] = &drv_usbd_hs_msc;
#endif //(1 == USBD_HS_ENABLED)
#endif // USBD_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_Init_(void* args_p)
{
const OS_UsbhHd* usbd_hd_p = (OS_UsbhHd*)args_p;
Status s = S_OK;
// Interface
#if (1 == USBD_FS_ENABLED)
    usbd_fs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->usbd_fs_hd;
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, USBD_UserProcess, USBD_ID_FS))   { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    usbd_hs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->usbd_hs_hd;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, USBD_UserProcess, USBD_ID_HS))   { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED

// Class
#if (1 == USBD_HID_ENABLED)
#if (1 == USBD_FS_ENABLED)
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED
#endif // USBD_HID_ENABLED

#if (1 == USBD_MSC_ENABLED)
#if (1 == USBD_FS_ENABLED)
    if (S_OK != drv_usbd_v[DRV_ID_USBD_FS_MSC]->Init(usbd_fs_hd_p))         { return s = S_HARDWARE_FAULT; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_MSC_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    if (S_OK != drv_usbd_v[DRV_ID_USBD_HS_MSC]->Init(usbd_hs_hd_p))         { return s = S_HARDWARE_FAULT; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_MSC_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED
#endif // USBD_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_DeInit_(void* args_p)
{
Status s = S_OK;
#if (1 == USBD_FS_ENABLED)
    HAL_HCD_MspDeInit(usbd_fs_hd_p->pData);
    if (USBD_OK != USBD_DeInit(usbd_fs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    HAL_HCD_MspDeInit(usbd_hs_hd_p->pData);
    if (USBD_OK != USBD_DeInit(usbd_hs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_Open(void* args_p)
{
Status s = S_OK;
    usbdd_stdin_qhd = (OS_QueueHd)args_p;
#if (1 == USBD_FS_ENABLED)
    if (USBD_OK != USBD_Start(usbd_fs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    if (USBD_OK != USBD_Start(usbd_hs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_Close(void* args_p)
{
Status s = S_OK;
    usbdd_stdin_qhd = OS_NULL;
#if (1 == USBD_FS_ENABLED)
    if (USBD_OK != USBD_Stop(usbd_fs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_FS_ENABLED
#if (1 == USBD_HS_ENABLED)
    if (USBD_OK != USBD_Stop(usbd_hs_hd_p)) { return s = S_HARDWARE_FAULT; }
#endif // USBD_HS_ENABLED
    return s;
}

///******************************************************************************/
//Status USBD_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBD_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBD_IT_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBD_IT_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBD_DMA_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBD_DMA_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}

/******************************************************************************/
Status USBD_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
                    break;
                default:
                    break;
            }
            if (HAL_OK != hal_status) { s = S_HARDWARE_FAULT; }
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

#endif //(1 == USBD_ENABLED)