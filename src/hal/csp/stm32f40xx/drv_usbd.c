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
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, &usbd_fs_hid_desc, OS_USB_ID_FS)){ return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    extern USBD_DescriptorsTypeDef usbd_hs_hid_desc;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, &usbd_hs_hid_desc, OS_USB_ID_HS)){ return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_HS_ENABLED)
#endif //(HAL_USBD_HID_ENABLED)

#if (HAL_USBD_MSC_ENABLED)
#if (HAL_USBD_FS_ENABLED)
    extern USBD_StorageTypeDef      usbd_fs_msc_itf;
    extern USBD_DescriptorsTypeDef  usbd_fs_msc_desc;
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, &usbd_fs_msc_desc, OS_USB_ID_FS)){ return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_MSC_CLASS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_MSC_RegisterStorage(usbd_fs_hd_p, &usbd_fs_msc_itf)){ return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBD_FS_ENABLED)
#if (HAL_USBD_HS_ENABLED)
    extern USBD_StorageTypeDef      usbd_hs_msc_itf;
    extern USBD_DescriptorsTypeDef  usbd_hs_msc_desc;
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, &usbd_hs_msc_desc, OS_USB_ID_HS)){ return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_MSC_CLASS))        { return s = S_HARDWARE_ERROR; }
    if (USBD_OK != USBD_MSC_RegisterStorage(usbd_hs_hd_p, &usbd_hs_msc_itf)){ return s = S_HARDWARE_ERROR; }
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
        __GPIOA_CLK_ENABLE();
        __GPIOB_CLK_ENABLE();
        __GPIOC_CLK_ENABLE();
        /**USB_OTG_FS GPIO Configuration
        PA9      ------> USB_OTG_FS_VBUS
        PA11     ------> USB_OTG_FS_DM
        PA12     ------> USB_OTG_FS_DP
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __USB_OTG_FS_CLK_ENABLE();

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(OTG_FS_IRQn, IRQ_PRIO_OTG_FS, 0);
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (USB_OTG_HS == hpcd->Instance) {
        __GPIOB_CLK_ENABLE();
        __GPIOD_CLK_ENABLE();
        __GPIOE_CLK_ENABLE();
        /**USB_OTG_HS GPIO Configuration
        PB12     ------> USB_OTG_HS_ID
        PB13     ------> USB_OTG_HS_VBUS
        PB14     ------> USB_OTG_HS_DM
        PB15     ------> USB_OTG_HS_DP
        */
        GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_OTG_HS_FS;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //    GPIO_InitStruct.Pin = GPIO_PIN_3;
    //    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    //    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __USB_OTG_HS_CLK_ENABLE();

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(OTG_HS_IRQn, HAL_IRQ_PRIO_OTG_HS, 0);
        HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
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
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

        /* Peripheral clock disable */
        __USB_OTG_FS_CLK_DISABLE();

        /**USB_OTG_FS GPIO Configuration
        PA9      ------> USB_OTG_FS_VBUS
        PA11     ------> USB_OTG_FS_DM
        PA12     ------> USB_OTG_FS_DP
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (USB_OTG_HS == hpcd->Instance) {
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(OTG_HS_IRQn);

        /* Peripheral clock disable */
        __USB_OTG_HS_CLK_DISABLE();

        /**USB_OTG_HS GPIO Configuration
        PB12     ------> USB_OTG_HS_ID
        PB13     ------> USB_OTG_HS_VBUS
        PB14     ------> USB_OTG_HS_DM
        PB15     ------> USB_OTG_HS_DP
        */
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_13);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
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
        pcd_fs_hd.Instance = USB_OTG_FS;
        pcd_fs_hd.Init.dev_endpoints = 4;
        pcd_fs_hd.Init.use_dedicated_ep1 = DISABLE;
        pcd_fs_hd.Init.ep0_mps = 0x40;
        pcd_fs_hd.Init.dma_enable = DISABLE;
        pcd_fs_hd.Init.low_power_enable = DISABLE;
        pcd_fs_hd.Init.phy_itface = PCD_PHY_EMBEDDED;
        pcd_fs_hd.Init.Sof_enable = DISABLE;
        pcd_fs_hd.Init.speed = PCD_SPEED_FULL;
        pcd_fs_hd.Init.vbus_sensing_enable = ENABLE;
        /* Link The driver to the stack */
        pcd_fs_hd.pData = pdev;
        pdev->pData = &pcd_fs_hd;
        /* Initialize LL Driver */
        HAL_PCD_Init(&pcd_fs_hd);

        HAL_PCD_SetRxFiFo(&pcd_fs_hd, 0x80);
        HAL_PCD_SetTxFiFo(&pcd_fs_hd, 0, 0x40);
        HAL_PCD_SetTxFiFo(&pcd_fs_hd, 1, 0x80);
    }
#endif //(HAL_USBD_FS_ENABLED)

#if (HAL_USBD_HS_ENABLED)
    if (OS_USB_ID_HS == pdev->id) {
        /* Set LL Driver parameters */
        pcd_hs_hd.Instance = USB_OTG_HS;
        pcd_hs_hd.Init.dev_endpoints = 6;
        pcd_hs_hd.Init.use_dedicated_ep1 = DISABLE;
        pcd_hs_hd.Init.ep0_mps = 0x40;

        /* Be aware that enabling DMA mode will result in data being sent only by
         multiple of 4 packet sizes. This is due to the fact that USB DMA does
         not allow sending data from non word-aligned addresses.
         For this specific application, it is advised to not enable this option
         unless required. */
        pcd_hs_hd.Init.dma_enable = ENABLE;

        pcd_hs_hd.Init.low_power_enable = DISABLE;
        pcd_hs_hd.Init.phy_itface = PCD_PHY_EMBEDDED;
        pcd_hs_hd.Init.Sof_enable = DISABLE;
        pcd_hs_hd.Init.speed = PCD_SPEED_FULL;
        pcd_hs_hd.Init.vbus_sensing_enable = ENABLE;
        /* Link The driver to the stack */
        pcd_hs_hd.pData = pdev;
        pdev->pData = &pcd_hs_hd;
        /* Initialize LL Driver */
        HAL_PCD_Init(&pcd_hs_hd);

        HAL_PCD_SetRxFiFo(&pcd_hs_hd, 0x200);
        HAL_PCD_SetTxFiFo(&pcd_hs_hd, 0, 0x80);
        HAL_PCD_SetTxFiFo(&pcd_hs_hd, 1, 0x174);
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