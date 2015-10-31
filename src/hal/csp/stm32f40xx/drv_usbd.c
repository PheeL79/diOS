/**************************************************************************//**
* @file    drv_usbd.c
* @brief   USB Device driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_mailbox.h"
#include "os_signal.h"
#include "os_debug.h"
#if (HAL_USBD_ENABLED)
#include "usbd_core.h"
#if (HAL_USBD_AUDIO_ENABLED)
    #include "usbd_audio.h"
#endif //(HAL_USBD_AUDIO_ENABLED)
#if (HAL_USBD_HID_ENABLED)
    #include "usbd_hid.h"
#endif //(HAL_USBD_HID_ENABLED)
#if (HAL_USBD_MSC_ENABLED)
    #include "usbd_msc.h"
#endif //(HAL_USBD_MSC_ENABLED)

//------------------------------------------------------------------------------
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
#if (HAL_USBD_FS_ENABLED)
PCD_HandleTypeDef           pcd_fs_hd;
static  USBD_HandleTypeDef* usbd_fs_hd_p;
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
PCD_HandleTypeDef           pcd_hs_hd;
static  USBD_HandleTypeDef* usbd_hs_hd_p;
#endif //(HAL_USBD_HS_ENABLED)
OS_QueueHd                  usbdd_stdin_qhd;
HAL_DriverItf*              drv_usbd_v[DRV_ID_USBX_LAST];

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
    HAL_MemSet(drv_usbd_v, 0x0, sizeof(drv_usbd_v));
    drv_usbd_v[DRV_ID_USBD] = &drv_usbd;
    return s;
}

/******************************************************************************/
Status USBD_Init_(void* args_p)
{
const OS_UsbDItfHd* usbd_hd_p = (OS_UsbDItfHd*)args_p;
Status s = S_OK;
// Interface
#if (HAL_USBD_FS_ENABLED)
    usbd_fs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->itf_fs_hd;
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    usbd_hs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->itf_hs_hd;
#endif //(HAL_USBD_HS_ENABLED)

// Class
#if (HAL_USBD_AUDIO_ENABLED)
#if (HAL_USBD_FS_ENABLED)
    extern USBD_AUDIO_ItfTypeDef    usbd_fs_audio_itf;
    extern USBD_DescriptorsTypeDef  usbd_fs_audio_desc;
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, &usbd_fs_audio_desc, OS_USB_ID_FS))      { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_AUDIO_CLASS))              { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_AUDIO_RegisterInterface(usbd_fs_hd_p, &usbd_fs_audio_itf))  { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    extern USBD_AUDIO_ItfTypeDef    usbd_hs_audio_itf;
    extern USBD_DescriptorsTypeDef  usbd_hs_audio_desc;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, &usbd_hs_audio_desc, OS_USB_ID_HS))      { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_AUDIO_CLASS))              { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_AUDIO_RegisterInterface(usbd_hs_hd_p, &usbd_hs_audio_itf))  { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
#endif //(HAL_USBD_AUDIO_ENABLED)

#if (HAL_USBD_HID_ENABLED)
#if (HAL_USBD_FS_ENABLED)
    extern USBD_DescriptorsTypeDef usbd_fs_hid_desc;
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, &usbd_fs_hid_desc, OS_USB_ID_FS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_HID_CLASS))                { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    extern USBD_DescriptorsTypeDef usbd_hs_hid_desc;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, &usbd_hs_hid_desc, OS_USB_ID_HS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_HID_CLASS))                { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
#endif //(HAL_USBD_HID_ENABLED)

#if (HAL_USBD_MSC_ENABLED)
#if (HAL_USBD_FS_ENABLED)
    extern USBD_StorageTypeDef      usbd_fs_msc_itf;
    extern USBD_DescriptorsTypeDef  usbd_fs_msc_desc;
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, &usbd_fs_msc_desc, OS_USB_ID_FS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_MSC_CLASS))                { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_MSC_RegisterStorage(usbd_fs_hd_p, &usbd_fs_msc_itf))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    extern USBD_StorageTypeDef      usbd_hs_msc_itf;
    extern USBD_DescriptorsTypeDef  usbd_hs_msc_desc;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, &usbd_hs_msc_desc, OS_USB_ID_HS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_MSC_CLASS))                { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_MSC_RegisterStorage(usbd_hs_hd_p, &usbd_hs_msc_itf))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
#endif //(HAL_USBD_MSC_ENABLED)
    return s;
}

/******************************************************************************/
Status USBD_DeInit_(void* args_p)
{
Status s = S_OK;
#if (HAL_USBD_FS_ENABLED)
    HAL_PCD_MspDeInit(usbd_fs_hd_p->pData);
    if (USBD_OK != USBD_DeInit(usbd_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    HAL_PCD_MspDeInit(usbd_hs_hd_p->pData);
    if (USBD_OK != USBD_DeInit(usbd_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
    return s;
}

/******************************************************************************/
Status USBD_Open(void* args_p)
{
Status s = S_OK;
    usbdd_stdin_qhd = (OS_QueueHd)args_p;
#if (HAL_USBD_FS_ENABLED)
    if (USBD_OK != USBD_Start(usbd_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    if (USBD_OK != USBD_Start(usbd_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
    return s;
}

/******************************************************************************/
Status USBD_Close(void* args_p)
{
Status s = S_OK;
    usbdd_stdin_qhd = OS_NULL;
#if (HAL_USBD_FS_ENABLED)
    if (USBD_OK != USBD_Stop(usbd_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    if (USBD_OK != USBD_Stop(usbd_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
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
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
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
            if (HAL_OK != hal_status) { s = S_HARDWARE_ERROR; }
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
}

/**
  * @brief  Initializes the PCD MSP.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
GPIO_InitTypeDef GPIO_InitStruct;

#if (HAL_USBD_FS_ENABLED)
    if (USB_OTG_FS == hpcd->Instance) {
        HAL_USB_OTG_FS_GPIO_CLK_ENABLE();

        GPIO_InitStruct.Pin         = HAL_USB_OTG_FS_GPIO1_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_FS_GPIO1_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_FS_GPIO1_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_FS_GPIO1_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_FS_GPIO1_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_FS_GPIO1_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = HAL_USB_OTG_FS_GPIO2_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_FS_GPIO2_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_FS_GPIO2_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_FS_GPIO2_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_FS_GPIO2_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_FS_GPIO2_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = HAL_USB_OTG_FS_GPIO3_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_FS_GPIO3_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_FS_GPIO3_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_FS_GPIO3_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_FS_GPIO3_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_FS_GPIO3_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = HAL_USB_OTG_FS_GPIO4_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_FS_GPIO4_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_FS_GPIO4_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_FS_GPIO4_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_FS_GPIO4_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_FS_GPIO4_PORT, &GPIO_InitStruct);

        /* Peripheral clock enable */
        HAL_USB_OTG_FS_CLK_ENABLE();

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(HAL_USB_OTG_FS_IRQ, HAL_PRIO_IRQ_USB_OTG_FS, 0);
        HAL_NVIC_EnableIRQ(HAL_USB_OTG_FS_IRQ);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (USB_OTG_HS == hpcd->Instance) {
        HAL_USB_OTG_HS_GPIO_CLK_ENABLE();

        GPIO_InitStruct.Pin         = HAL_USB_OTG_HS_GPIO1_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_HS_GPIO1_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_HS_GPIO1_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_HS_GPIO1_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_HS_GPIO1_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_HS_GPIO1_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = HAL_USB_OTG_HS_GPIO2_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_HS_GPIO2_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_HS_GPIO2_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_HS_GPIO2_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_HS_GPIO2_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_HS_GPIO2_PORT, &GPIO_InitStruct);

//        GPIO_InitStruct.Pin         = HAL_USB_OTG_HS_GPIO3_PIN;
//        GPIO_InitStruct.Mode        = HAL_USB_OTG_HS_GPIO3_MODE;
//        GPIO_InitStruct.Pull        = HAL_USB_OTG_HS_GPIO3_PULL;
//        GPIO_InitStruct.Speed       = HAL_USB_OTG_HS_GPIO3_SPEED;
//        GPIO_InitStruct.Alternate   = HAL_USB_OTG_HS_GPIO3_ALT;
//        HAL_GPIO_Init(HAL_USB_OTG_HS_GPIO3_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = HAL_USB_OTG_HS_GPIO4_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_HS_GPIO4_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_HS_GPIO4_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_HS_GPIO4_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_HS_GPIO4_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_HS_GPIO4_PORT, &GPIO_InitStruct);

        /* Peripheral clock enable */
        HAL_USB_OTG_HS_CLK_ENABLE();

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(HAL_USB_OTG_HS_IRQ, HAL_PRIO_IRQ_USB_OTG_HS, 0);
        HAL_NVIC_EnableIRQ(HAL_USB_OTG_HS_IRQ);
    }
#endif //(HAL_USBD_HS_ENABLED)
}

/**
  * @brief  De-Initializes the PCD MSP.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
#if (HAL_USBD_FS_ENABLED)
    if (USB_OTG_FS == hpcd->Instance) {
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(HAL_USB_OTG_FS_IRQ);

        /* Peripheral clock disable */
        HAL_USB_OTG_FS_CLK_DISABLE();

        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO1_PORT, HAL_USB_OTG_FS_GPIO1_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO2_PORT, HAL_USB_OTG_FS_GPIO2_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO3_PORT, HAL_USB_OTG_FS_GPIO3_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO4_PORT, HAL_USB_OTG_FS_GPIO4_PIN);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (USB_OTG_HS == hpcd->Instance) {
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(HAL_USB_OTG_HS_IRQ);

        /* Peripheral clock disable */
        HAL_USB_OTG_HS_CLK_DISABLE();

        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO1_PORT, HAL_USB_OTG_HS_GPIO1_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO2_PORT, HAL_USB_OTG_HS_GPIO2_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO3_PORT, HAL_USB_OTG_HS_GPIO3_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO4_PORT, HAL_USB_OTG_HS_GPIO4_PIN);
    }
#endif //(HAL_USBD_HS_ENABLED)
}

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetupStage(hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage(hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage(hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF(hpcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

  /* Set USB Current Speed */
  switch(hpcd->Init.speed)
  {
  case PCD_SPEED_HIGH:
    speed = USBD_SPEED_HIGH;
    break;

  case PCD_SPEED_FULL:
    speed = USBD_SPEED_FULL;
    break;

  default:
    speed = USBD_SPEED_FULL;
    break;
  }
  USBD_LL_SetSpeed(hpcd->pData, speed);

  /* Reset Device */
  USBD_LL_Reset(hpcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Suspend(hpcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Resume(hpcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
USBD_HandleTypeDef* pdev = hpcd->pData;
    USBD_LL_DevConnected(pdev);
    OS_SignalData signal_data = 0;
    OS_USB_SIG_ITF_SET(signal_data, pdev->id);
    OS_USB_SIG_MSG_SET(signal_data, OS_SIG_USB_CONNECT);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBD, OS_SIG_DRV, signal_data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbdd_stdin_qhd, signal, OS_MSG_PRIO_HIGH));
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
USBD_HandleTypeDef* pdev = hpcd->pData;
    USBD_LL_DevDisconnected(pdev);
    OS_SignalData signal_data = 0;
    OS_USB_SIG_ITF_SET(signal_data, pdev->id);
    OS_USB_SIG_MSG_SET(signal_data, OS_SIG_USB_DISCONNECT);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBD, OS_SIG_DRV, signal_data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbdd_stdin_qhd, signal, OS_MSG_PRIO_HIGH));
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
#if (HAL_USBD_FS_ENABLED)
    if (OS_USB_ID_FS == pdev->id) {
        /* Set LL Driver parameters */
        pcd_fs_hd.Instance                  = HAL_USB_OTG_FS_ITF;
        pcd_fs_hd.Init.dev_endpoints        = HAL_USBD_OTG_FS_DEV_ENDPOINTS;
        pcd_fs_hd.Init.use_dedicated_ep1    = HAL_USBD_OTG_FS_USE_DED_EP1;
        pcd_fs_hd.Init.ep0_mps              = HAL_USBD_OTG_FS_EP0_MPS;
        pcd_fs_hd.Init.dma_enable           = HAL_USBD_OTG_FS_DMA_ENABLE;
        pcd_fs_hd.Init.low_power_enable     = HAL_USBD_OTG_FS_LOW_POWER_ENABLE;
        pcd_fs_hd.Init.phy_itface           = HAL_USBD_OTG_FS_PHY_ITF;
        pcd_fs_hd.Init.Sof_enable           = HAL_USBD_OTG_FS_SOF_ENABLE;
        pcd_fs_hd.Init.speed                = HAL_USBD_OTG_FS_SPEED;
        pcd_fs_hd.Init.vbus_sensing_enable  = HAL_USBD_OTG_FS_VBUS_SENS_ENABLE;
        /* Link The driver to the stack */
        pcd_fs_hd.pData = pdev;
        pdev->pData = &pcd_fs_hd;
        /* Initialize LL Driver */
        HAL_PCD_Init(&pcd_fs_hd);

        HAL_PCD_SetRxFiFo(&pcd_fs_hd, HAL_USBD_OTG_FS_FIFO_SIZE_RX);
        HAL_PCD_SetTxFiFo(&pcd_fs_hd, 0, HAL_USBD_OTG_FS_FIFO_SIZE_TX_0);
        HAL_PCD_SetTxFiFo(&pcd_fs_hd, 1, HAL_USBD_OTG_FS_FIFO_SIZE_TX_1);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (OS_USB_ID_HS == pdev->id) {
        /* Set LL Driver parameters */
        pcd_hs_hd.Instance                  = HAL_USB_OTG_HS_ITF;
        pcd_hs_hd.Init.dev_endpoints        = HAL_USBD_OTG_HS_DEV_ENDPOINTS;
        pcd_hs_hd.Init.use_dedicated_ep1    = HAL_USBD_OTG_HS_USE_DED_EP1;
        pcd_hs_hd.Init.ep0_mps              = HAL_USBD_OTG_HS_EP0_MPS;
        pcd_hs_hd.Init.dma_enable           = HAL_USBD_OTG_HS_DMA_ENABLE;
        pcd_hs_hd.Init.low_power_enable     = HAL_USBD_OTG_HS_LOW_POWER_ENABLE;
        pcd_hs_hd.Init.phy_itface           = HAL_USBD_OTG_HS_PHY_ITF;
        pcd_hs_hd.Init.Sof_enable           = HAL_USBD_OTG_HS_SOF_ENABLE;
        pcd_hs_hd.Init.speed                = HAL_USBD_OTG_HS_SPEED;
        pcd_hs_hd.Init.vbus_sensing_enable  = HAL_USBD_OTG_HS_VBUS_SENS_ENABLE;
        /* Link The driver to the stack */
        pcd_hs_hd.pData = pdev;
        pdev->pData = &pcd_hs_hd;
        /* Initialize LL Driver */
        HAL_PCD_Init(&pcd_hs_hd);

        HAL_PCD_SetRxFiFo(&pcd_hs_hd, HAL_USBD_OTG_HS_FIFO_SIZE_RX);
        HAL_PCD_SetTxFiFo(&pcd_hs_hd, 0, HAL_USBD_OTG_HS_FIFO_SIZE_TX_0);
        HAL_PCD_SetTxFiFo(&pcd_hs_hd, 1, HAL_USBD_OTG_HS_FIFO_SIZE_TX_1);
    }
#endif //(HAL_USBD_HS_ENABLED)

  return USBD_OK;
}

/**
  * @brief  De-Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_DeInit(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Starts the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Start(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Stops the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Stop(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Opens an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  ep_type: Endpoint Type
  * @param  ep_mps: Endpoint Max Packet Size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev,
                                  uint8_t ep_addr,
                                  uint8_t ep_type,
                                  uint16_t ep_mps)
{
  HAL_PCD_EP_Open(pdev->pData,
                  ep_addr,
                  ep_mps,
                  ep_type);

  return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Close(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Flush(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = pdev->pData;

  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  HAL_PCD_SetAddress(pdev->pData, dev_addr);
  return USBD_OK;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev,
                                    uint8_t ep_addr,
                                    uint8_t *pbuf,
                                    uint16_t size)
{
  HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
                                          uint8_t ep_addr,
                                          uint8_t *pbuf,
                                          uint16_t size)
{
  HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Returns the last transfered packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Recived Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount(pdev->pData, ep_addr);
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

#endif //(HAL_USBD_ENABLED)