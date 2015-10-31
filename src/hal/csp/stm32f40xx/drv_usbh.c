/**************************************************************************//**
* @file    drv_usbh.c
* @brief   USB Host driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_mailbox.h"
#include "os_signal.h"
#include "os_debug.h"
#if (HAL_USBH_ENABLED)
#include "usbh_core.h"
#if (HAL_USBH_HID_ENABLED)
    #include "usbh_hid.h"
#endif //(HAL_USBH_HID_ENABLED)
#if (HAL_USBH_MSC_ENABLED)
    #include "usbh_msc.h"
#endif //(HAL_USBH_MSC_ENABLED)

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usbh"

//------------------------------------------------------------------------------
/// @brief   USBH initialization.
/// @return  #Status.
Status USBH__Init(void);

static Status   USBH_Init_(void* args_p);
static Status   USBH_DeInit_(void* args_p);
static Status   USBH_Open(void* args_p);
static Status   USBH_Close(void* args_p);
static Status   USBH_IoCtl(const U32 request_id, void* args_p);

static void     USBH_UserProcess(USBH_HandleTypeDef* usbh_itf_hd, uint8_t id);

//------------------------------------------------------------------------------
#if (HAL_USBH_FS_ENABLED)
HCD_HandleTypeDef           hcd_fs_hd;
static USBH_HandleTypeDef*  usbh_fs_hd_p;
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
HCD_HandleTypeDef           hcd_hs_hd;
static USBH_HandleTypeDef*  usbh_hs_hd_p;
#endif //(HAL_USBH_HS_ENABLED)
OS_QueueHd                  usbhd_stdin_qhd;
HAL_DriverItf*              drv_usbh_v[DRV_ID_USBX_LAST];

static HAL_DriverItf drv_usbh = {
    .Init   = USBH_Init_,
    .DeInit = USBH_DeInit_,
    .Open   = USBH_Open,
    .Close  = USBH_Close,
    .IoCtl  = USBH_IoCtl
};

/******************************************************************************/
Status USBH__Init(void)
{
Status s = S_OK;
    HAL_MemSet(drv_usbh_v, 0x0, sizeof(drv_usbh_v));
    drv_usbh_v[DRV_ID_USBH] = &drv_usbh;
    return s;
}

/******************************************************************************/
Status USBH_Init_(void* args_p)
{
const OS_UsbHItfHd* usbh_itf_hd = (OS_UsbHItfHd*)args_p;
Status s = S_OK;
// Interface
#if (HAL_USBH_FS_ENABLED)
    usbh_fs_hd_p = (USBH_HandleTypeDef*)usbh_itf_hd->itf_fs_hd;
    if (USBH_OK != USBH_Init(usbh_fs_hd_p, USBH_UserProcess, OS_USB_ID_FS))   { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    usbh_hs_hd_p = (USBH_HandleTypeDef*)usbh_itf_hd->itf_hs_hd;
    if (USBH_OK != USBH_Init(usbh_hs_hd_p, USBH_UserProcess, OS_USB_ID_HS))   { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)

// Class
#if (HAL_USBH_HID_ENABLED)
#if (HAL_USBH_FS_ENABLED)
    if (USBH_OK != USBH_RegisterClass(usbh_fs_hd_p, USBH_HID_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    if (USBH_OK != USBH_RegisterClass(usbh_hs_hd_p, USBH_HID_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)
#endif //(HAL_USBH_HID_ENABLED)

#if (HAL_USBH_MSC_ENABLED)
#if (HAL_USBH_FS_ENABLED)
    if (USBH_OK != USBH_RegisterClass(usbh_fs_hd_p, USBH_MSC_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    if (USBH_OK != USBH_RegisterClass(usbh_hs_hd_p, USBH_MSC_CLASS))        { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)
#endif //(HAL_USBH_MSC_ENABLED)
    return s;
}

/******************************************************************************/
Status USBH_DeInit_(void* args_p)
{
Status s = S_OK;
#if (HAL_USBH_FS_ENABLED)
    HAL_HCD_MspDeInit(usbh_fs_hd_p->pData);
    if (USBH_OK != USBH_DeInit(usbh_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    HAL_HCD_MspDeInit(usbh_hs_hd_p->pData);
    if (USBH_OK != USBH_DeInit(usbh_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)
    return s;
}

/******************************************************************************/
Status USBH_Open(void* args_p)
{
Status s = S_OK;
    usbhd_stdin_qhd = (OS_QueueHd)args_p;
#if (HAL_USBH_FS_ENABLED)
    if (USBH_OK != USBH_Start(usbh_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    if (USBH_OK != USBH_Start(usbh_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)
    return s;
}

/******************************************************************************/
Status USBH_Close(void* args_p)
{
Status s = S_OK;
    usbhd_stdin_qhd = OS_NULL;
#if (HAL_USBH_FS_ENABLED)
    if (USBH_OK != USBH_Stop(usbh_fs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_FS_ENABLED)
#if (HAL_USBH_HS_ENABLED)
    if (USBH_OK != USBH_Stop(usbh_hs_hd_p)) { return s = S_HARDWARE_ERROR; }
#endif //(HAL_USBH_HS_ENABLED)
    return s;
}

///******************************************************************************/
//Status USBH_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBH_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBH_IT_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBH_IT_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBH_DMA_Read(U8* data_in_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}
//
///******************************************************************************/
//Status USBH_DMA_Write(U8* data_out_p, U32 size, void* args_p)
//{
//Status s = S_OK;
//    return s;
//}

/******************************************************************************/
Status USBH_IoCtl(const U32 request_id, void* args_p)
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
/*******************************************************************************
                       LL Driver Callbacks (HCD -> USB Host Library)
*******************************************************************************/
/* MSP Init */

void HAL_HCD_MspInit(HCD_HandleTypeDef* hhcd)
{
GPIO_InitTypeDef GPIO_InitStruct;
#if (HAL_USBH_FS_ENABLED)
    if (USB_OTG_FS == hhcd->Instance) {
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
#endif //(HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
    if (USB_OTG_HS == hhcd->Instance) {
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

        GPIO_InitStruct.Pin         = HAL_USB_OTG_HS_GPIO3_PIN;
        GPIO_InitStruct.Mode        = HAL_USB_OTG_HS_GPIO3_MODE;
        GPIO_InitStruct.Pull        = HAL_USB_OTG_HS_GPIO3_PULL;
        GPIO_InitStruct.Speed       = HAL_USB_OTG_HS_GPIO3_SPEED;
        GPIO_InitStruct.Alternate   = HAL_USB_OTG_HS_GPIO3_ALT;
        HAL_GPIO_Init(HAL_USB_OTG_HS_GPIO3_PORT, &GPIO_InitStruct);

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
#endif //(HAL_USBH_FS_ENABLED)
}

void HAL_HCD_MspDeInit(HCD_HandleTypeDef* hhcd)
{
#if (HAL_USBH_FS_ENABLED)
    if (USB_OTG_FS == hhcd->Instance) {
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(HAL_USB_OTG_FS_IRQ);

        /* Peripheral clock disable */
        HAL_USB_OTG_FS_CLK_DISABLE();

        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO1_PORT, HAL_USB_OTG_FS_GPIO1_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO2_PORT, HAL_USB_OTG_FS_GPIO2_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO3_PORT, HAL_USB_OTG_FS_GPIO3_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_FS_GPIO4_PORT, HAL_USB_OTG_FS_GPIO4_PIN);
    }
#endif //(HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
    if (USB_OTG_HS == hhcd->Instance) {
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(HAL_USB_OTG_HS_IRQ);

        /* Peripheral clock disable */
        HAL_USB_OTG_HS_CLK_DISABLE();

        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO1_PORT, HAL_USB_OTG_HS_GPIO1_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO2_PORT, HAL_USB_OTG_HS_GPIO2_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO3_PORT, HAL_USB_OTG_HS_GPIO3_PIN);
        HAL_GPIO_DeInit(HAL_USB_OTG_HS_GPIO4_PORT, HAL_USB_OTG_HS_GPIO4_PIN);
    }
#endif //(HAL_USBH_HS_ENABLED)
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_IncTimer (hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_Connect(hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_Disconnect(hhcd->pData);
}

/**
  * @brief  Notify URB state change callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
  /* To be used with OS to sync URB state with the global state machine */
#if (USBH_USE_OS)
  USBH_LL_NotifyURBChange(hhcd->pData);
#endif
#if (USBH_USE_diOS)
OS_SignalData signal_data = 0;
    OS_USB_SIG_ITF_SET(signal_data, ((USBH_HandleTypeDef*)(hhcd->pData))->id);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBH, OS_SIG_DRV, signal_data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbhd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
#endif //(USBH_USE_diOS)
}
/*******************************************************************************
                       LL Driver Interface (USB Host Library --> HCD)
*******************************************************************************/
/**
  * @brief  USBH_LL_Init
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
USBH_StatusTypeDef  USBH_LL_Init (USBH_HandleTypeDef *phost)
{
  /* Init USB_IP */
#if (HAL_USBH_FS_ENABLED)
    if (OS_USB_ID_FS == phost->id) {
        hcd_fs_hd.Instance                  = HAL_USB_OTG_FS_ITF;
        hcd_fs_hd.Init.Host_channels        = HAL_USBH_OTG_FS_HOST_CHANNELS;
        hcd_fs_hd.Init.speed                = HAL_USBH_OTG_FS_SPEED;
        hcd_fs_hd.Init.dma_enable           = HAL_USBH_OTG_FS_DMA_ENABLE;
        hcd_fs_hd.Init.phy_itface           = HAL_USBH_OTG_FS_PHY_ITF;
        hcd_fs_hd.Init.Sof_enable           = HAL_USBH_OTG_FS_SOF_ENABLE;
        hcd_fs_hd.Init.low_power_enable     = HAL_USBH_OTG_FS_LOW_POWER_ENABLE;
        hcd_fs_hd.Init.vbus_sensing_enable  = HAL_USBH_OTG_FS_VBUS_SENS_ENABLE;
        hcd_fs_hd.Init.use_external_vbus    = HAL_USBH_OTG_FS_VBUS_EXT_ENABLE;
        /* Link The driver to the stack */
        hcd_fs_hd.pData = phost;
        phost->pData = &hcd_fs_hd;
        HAL_HCD_Init(&hcd_fs_hd);
        USBH_LL_SetTimer (phost, HAL_HCD_GetCurrentFrame(&hcd_fs_hd));
    }
#endif //(HAL_USBH_FS_ENABLED)

#if (HAL_USBH_HS_ENABLED)
    if (OS_USB_ID_HS == phost->id) {
        hcd_hs_hd.Instance                  = HAL_USB_OTG_HS_ITF;
        hcd_hs_hd.Init.Host_channels        = HAL_USBH_OTG_HS_HOST_CHANNELS;
        hcd_hs_hd.Init.speed                = HAL_USBH_OTG_HS_SPEED;
        hcd_hs_hd.Init.dma_enable           = HAL_USBH_OTG_HS_DMA_ENABLE;
        hcd_hs_hd.Init.phy_itface           = HAL_USBH_OTG_HS_PHY_ITF;
        hcd_hs_hd.Init.Sof_enable           = HAL_USBH_OTG_HS_SOF_ENABLE;
        hcd_hs_hd.Init.low_power_enable     = HAL_USBH_OTG_HS_LOW_POWER_ENABLE;
        hcd_hs_hd.Init.vbus_sensing_enable  = HAL_USBH_OTG_HS_VBUS_SENS_ENABLE;
        hcd_hs_hd.Init.use_external_vbus    = HAL_USBH_OTG_HS_VBUS_EXT_ENABLE;
        /* Link The driver to the stack */
        hcd_hs_hd.pData = phost;
        phost->pData = &hcd_hs_hd;
        HAL_HCD_Init(&hcd_hs_hd);
        USBH_LL_SetTimer (phost, HAL_HCD_GetCurrentFrame(&hcd_hs_hd));
    }
#endif //(HAL_USBH_HS_ENABLED)

  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef  USBH_LL_DeInit (USBH_HandleTypeDef *phost)
{
  HAL_HCD_DeInit(phost->pData);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef  USBH_LL_Start(USBH_HandleTypeDef *phost)
{
  HAL_HCD_Start(phost->pData);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef  USBH_LL_Stop (USBH_HandleTypeDef *phost)
{
  HAL_HCD_Stop(phost->pData);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_SpeedTypeDef USBH_LL_GetSpeed  (USBH_HandleTypeDef *phost)
{
  USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

  switch (HAL_HCD_GetCurrentSpeed(phost->pData))
  {
  case 0 :
    speed = USBH_SPEED_HIGH;
    break;

  case 1 :
    speed = USBH_SPEED_FULL;
    break;

  case 2 :
    speed = USBH_SPEED_LOW;
    break;

  default:
   speed = USBH_SPEED_FULL;
    break;
  }
  return  speed;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_ResetPort (USBH_HandleTypeDef *phost)
{
  HAL_HCD_ResetPort(phost->pData);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
uint32_t USBH_LL_GetLastXferSize  (USBH_HandleTypeDef *phost, uint8_t pipe)
{
  return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef   USBH_LL_OpenPipe    (USBH_HandleTypeDef *phost,
                                      uint8_t pipe_num,
                                      uint8_t epnum,
                                      uint8_t dev_address,
                                      uint8_t speed,
                                      uint8_t ep_type,
                                      uint16_t mps)
{
  HAL_HCD_HC_Init(phost->pData,
                  pipe_num,
                  epnum,
                  dev_address,
                  speed,
                  ep_type,
                  mps);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef   USBH_LL_ClosePipe   (USBH_HandleTypeDef *phost, uint8_t pipe)
{
  HAL_HCD_HC_Halt(phost->pData, pipe);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */

USBH_StatusTypeDef   USBH_LL_SubmitURB  (USBH_HandleTypeDef *phost,
                                            uint8_t pipe,
                                            uint8_t direction ,
                                            uint8_t ep_type,
                                            uint8_t token,
                                            uint8_t* pbuff,
                                            uint16_t length,
                                            uint8_t do_ping )
{
  HAL_HCD_HC_SubmitRequest (phost->pData,
                            pipe,
                            direction ,
                            ep_type,
                            token,
                            pbuff,
                            length,
                            do_ping);
  return USBH_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_URBStateTypeDef  USBH_LL_GetURBState (USBH_HandleTypeDef *phost, uint8_t pipe)
{
  return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState (phost->pData, pipe);
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBH_StatusTypeDef  USBH_LL_DriverVBUS (USBH_HandleTypeDef *phost, uint8_t state)
{
 /* USER CODE BEGIN 0 */
 /* USER CODE END 0 */
  if(state == 0)
  {
    /* Drive high Charge pump */
    /* USER CODE BEGIN 1 */
    /* ToDo: Add IOE driver control */
    /* USER CODE END 1 */
    if (phost->id == OS_USB_ID_FS)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    }
    else if (phost->id == OS_USB_ID_HS)
    {
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    }
  }
  else
  {
    /* Drive low Charge pump */
    /* USER CODE BEGIN 2 */
    /* ToDo: Add IOE driver control */
    /* USER CODE END 2 */
    if (phost->id == OS_USB_ID_FS)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    }
    else if (phost->id == OS_USB_ID_HS)
    {
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
    }
  }

  HAL_Delay(200);
  return USBH_OK;
}

/**
  * @brief  USBH_LL_SetToggle
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
USBH_StatusTypeDef   USBH_LL_SetToggle   (USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle)
{
  HCD_HandleTypeDef *pHandle;
  pHandle = phost->pData;

  if(pHandle->hc[pipe].ep_is_in)
  {
    pHandle->hc[pipe].toggle_in = toggle;
  }
  else
  {
    pHandle->hc[pipe].toggle_out = toggle;
  }

  return USBH_OK;
}

/**
  * @brief  USBH_LL_GetToggle
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
uint8_t  USBH_LL_GetToggle   (USBH_HandleTypeDef *phost, uint8_t pipe)
{
  uint8_t toggle = 0;
  HCD_HandleTypeDef *pHandle;
  pHandle = phost->pData;

  if(pHandle->hc[pipe].ep_is_in)
  {
    toggle = pHandle->hc[pipe].toggle_in;
  }
  else
  {
    toggle = pHandle->hc[pipe].toggle_out;
  }
  return toggle;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
void  USBH_Delay (uint32_t Delay)
{
    HAL_Delay(Delay);
}

/******************************************************************************/
/*
 * user callbañk definition
*/
void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
OS_SignalData signal_data = 0;
    OS_USB_SIG_ITF_SET(signal_data, phost->id);
    OS_USB_SIG_MSG_SET(signal_data, id);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBH, OS_SIG_DRV, signal_data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbhd_stdin_qhd, signal, OS_MSG_PRIO_HIGH));
}
#endif //(HAL_USBH_ENABLED)

// USBH/D IRQ handlers----------------------------------------------------------
// stm32f4xx_it.c
