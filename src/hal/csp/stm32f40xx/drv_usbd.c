/**************************************************************************//**
* @file    drv_usbd.c
* @brief   USB Device driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "usbd_core.h"
#if (1 == USBD_HID_ENABLED)
    #include "usbd_hid.h"
#endif // USBD_HID_ENABLED
#if (1 == USBD_MSC_ENABLED)
    #include "usbd_msc.h"
#endif // USBD_MSC_ENABLED
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_message.h"
#include "os_signal.h"
#include "os_debug.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usbd"

//------------------------------------------------------------------------------
/// @brief   USBD initialization.
/// @return  #Status.
Status USBD__Init(void);

static Status   USBD_Init_(void* args_p);
static Status   USBD_DeInit_(void);
static Status   USBD_Open(void* args_p);
static Status   USBD_Close(void);
static Status   USBD_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBD_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBD_IT_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBD_IT_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBD_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBD_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBD_IoCtl(const U32 request_id, void* args_p);

static void     USBD_UserProcess(USBD_HandleTypeDef* usbd_hd_p, uint8_t id);

//------------------------------------------------------------------------------
static HCD_HandleTypeDef    hcd_otg_hs_hd;
static USBD_HandleTypeDef*  usbd_hs_hd_p;
OS_QueueHd                  usbdd_stdin_qhd;
HAL_DriverItf*              drv_usb_fs_v[DRV_ID_USBD_LAST];
HAL_DriverItf*              drv_usb_hs_v[DRV_ID_USBD_LAST];

static HAL_DriverItf drv_usbd = {
    .Init   = USBD_Init_,
    .DeInit = USBD_DeInit_,
    .Open   = USBD_Open,
    .Close  = USBD_Close,
    .Read   = USBD_Read,
    .Write  = USBD_Write,
    .IoCtl  = USBD_IoCtl
};

/******************************************************************************/
Status USBD__Init(void)
{
Status s = S_OK;
    HAL_MEMSET(drv_usbd_v, 0x0, sizeof(drv_usbd_v));
    drv_usbd_v[DRV_ID_USBD]     = &drv_usbd;
#if (1 == USBD_MSC_ENABLED)
    extern HAL_DriverItf drv_usbd_msc;
    drv_usbd_v[DRV_ID_USBD_MSC] = &drv_usbd_msc;
#endif // USBD_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_Init_(void* args_p)
{
const OS_UsbhHd* usbd_hd_p = (OS_UsbhHd*)args_p;
Status s = S_OK;
    usbd_fs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->usbd_fs_hd;
    usbd_hs_hd_p = (USBD_HandleTypeDef*)usbd_hd_p->usbd_hs_hd;

// Interface
#if (1 == HOST_FS)
    if (USBD_OK != USBD_Init(usbd_fs_hd_p, USBD_UserProcess, USBD_ID_FS))   { return s = S_HARDWARE_FAULT; }
#endif // HOST_FS
#if (1 == HOST_HS)
    if (USBD_OK != USBD_Init(usbd_hs_hd_p, USBD_UserProcess, USBD_ID_HS))   { return s = S_HARDWARE_FAULT; }
#endif // HOST_HS

// Class
#if (1 == USBD_HID_ENABLED)
#if (1 == HOST_FS)
    if (USBD_OK != USBD_RegisterClass(usbd_fs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // HOST_FS
#if (1 == HOST_HS)
    if (USBD_OK != USBD_RegisterClass(usbd_hs_hd_p, USBD_HID_CLASS))        { return s = S_HARDWARE_FAULT; }
#endif // HOST_HS
#endif // USBD_HID_ENABLED

#if (1 == USBD_MSC_ENABLED)
    if (S_OK != drv_usbd_v[DRV_ID_USBD_MSC]->Init(usbd_hd_p))               { return s = S_HARDWARE_FAULT; }
    if (USBD_OK != USBD_RegisterClass(usbd_hd_p, USBD_MSC_CLASS))           { return s = S_HARDWARE_FAULT; }
#endif // USBD_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBD_DeInit_(void)
{
Status s = S_OK;
    HAL_HCD_MspDeInit(usbd_hd_p->pData);
    if (USBD_OK != USBD_DeInit(usbd_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBD_Open(void* args_p)
{
Status s = S_OK;
    usbdd_stdin_qhd = (OS_QueueHd)args_p;
    if (USBD_OK != USBD_Start(usbd_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBD_Close(void)
{
Status s = S_OK;
    usbdd_stdin_qhd = OS_NULL;
    if (USBD_OK != USBD_Stop(usbd_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBD_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBD_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBD_IT_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBD_IT_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBD_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBD_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

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
/*******************************************************************************
                       LL Driver Callbacks (HCD -> USB Host Library)
*******************************************************************************/
/* MSP Init */

void HAL_HCD_MspInit(HCD_HandleTypeDef* hhcd)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  if(hhcd->Instance==USB_OTG_FS)
  {
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
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __USB_OTG_FS_CLK_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(OTG_FS_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  }
  else if(hhcd->Instance == USB_OTG_HS)
  {
    __GPIOB_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    /**USB_OTG_HS GPIO Configuration
    PB12     ------> USB_OTG_HS_ID
    PB13     ------> USB_OTG_HS_VBUS
    PB14     ------> USB_OTG_HS_DM
    PB15     ------> USB_OTG_HS_DP
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __USB_OTG_HS_CLK_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(OTG_HS_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
  }
}

void HAL_HCD_MspDeInit(HCD_HandleTypeDef* hhcd)
{
  if(hhcd->Instance == USB_OTG_FS)
  {
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
  else if (hhcd->Instance == USB_OTG_HS)
  {
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
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{
  USBD_LL_IncTimer (hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBD_LL_Connect(hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBD_LL_Disconnect(hhcd->pData);
}

/**
  * @brief  Notify URB state change callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
  /* To be used with OS to sync URB state with the global state machine */
#if (USBD_USE_OS == 1)
  USBD_LL_NotifyURBChange(hhcd->pData);
#endif
#if (1 == USBD_USE_diOS)
const OS_Signal signal = OS_ISR_SIGNAL_CREATE(DRV_ID_USBD, OS_SIG_DRV, 0);
    if (1 == OS_ISR_SIGNAL_SEND(usbdd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL)) {
        OS_ContextSwitchForce();
    }
#endif // USBD_USE_diOS
}
/*******************************************************************************
                       LL Driver Interface (USB Host Library --> HCD)
*******************************************************************************/
/**
  * @brief  USBD_LL_Init
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
USBD_StatusTypeDef  USBD_LL_Init (USBD_HandleTypeDef *phost)
{
  /* Init USB_IP */
  if (phost->id == USBD_ID_FS)
  {
  hcd_otg_fs_hd.Instance = USB_OTG_FS;
  hcd_otg_fs_hd.Init.Host_channels = 8;
  hcd_otg_fs_hd.Init.speed = HCD_SPEED_FULL;
  hcd_otg_fs_hd.Init.dma_enable = DISABLE;
  hcd_otg_fs_hd.Init.phy_itface = HCD_PHY_EMBEDDED;
  hcd_otg_fs_hd.Init.Sof_enable = DISABLE;
  hcd_otg_fs_hd.Init.low_power_enable = ENABLE;
  hcd_otg_fs_hd.Init.vbus_sensing_enable = ENABLE;
  hcd_otg_fs_hd.Init.use_external_vbus = ENABLE;
  }
  else if (phost->id == USBD_ID_HS)
  {
  hcd_otg_hs_hd.Instance = USB_OTG_HS;
  hcd_otg_hs_hd.Init.Host_channels = 8;
  hcd_otg_hs_hd.Init.speed = HCD_SPEED_FULL;
  hcd_otg_hs_hd.Init.dma_enable = ENABLE;
  hcd_otg_hs_hd.Init.phy_itface = HCD_PHY_EMBEDDED;
  hcd_otg_hs_hd.Init.Sof_enable = DISABLE;
  hcd_otg_hs_hd.Init.low_power_enable = DISABLE;
  hcd_otg_hs_hd.Init.vbus_sensing_enable = ENABLE;
  hcd_otg_hs_hd.Init.use_external_vbus = ENABLE;
  }
  /* Link The driver to the stack */
  hcd_otg_hd.pData = phost;
  phost->pData = &hcd_otg_hd;
  HAL_HCD_Init(&hcd_otg_hd);
  USBD_LL_SetTimer (phost, HAL_HCD_GetCurrentFrame(&hcd_otg_hd));
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef  USBD_LL_DeInit (USBD_HandleTypeDef *phost)
{
  HAL_HCD_DeInit(phost->pData);
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef  USBD_LL_Start(USBD_HandleTypeDef *phost)
{
  HAL_HCD_Start(phost->pData);
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef  USBD_LL_Stop (USBD_HandleTypeDef *phost)
{
  HAL_HCD_Stop(phost->pData);
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_SpeedTypeDef USBD_LL_GetSpeed  (USBD_HandleTypeDef *phost)
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

  switch (HAL_HCD_GetCurrentSpeed(phost->pData))
  {
  case 0 :
    speed = USBD_SPEED_HIGH;
    break;

  case 1 :
    speed = USBD_SPEED_FULL;
    break;

  case 2 :
    speed = USBD_SPEED_LOW;
    break;

  default:
   speed = USBD_SPEED_FULL;
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
USBD_StatusTypeDef USBD_LL_ResetPort (USBD_HandleTypeDef *phost)
{
  HAL_HCD_ResetPort(phost->pData);
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
uint32_t USBD_LL_GetLastXferSize  (USBD_HandleTypeDef *phost, uint8_t pipe)
{
  return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef   USBD_LL_OpenPipe    (USBD_HandleTypeDef *phost,
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
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef   USBD_LL_ClosePipe   (USBD_HandleTypeDef *phost, uint8_t pipe)
{
  HAL_HCD_HC_Halt(phost->pData, pipe);
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */

USBD_StatusTypeDef   USBD_LL_SubmitURB  (USBD_HandleTypeDef *phost,
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
  return USBD_OK;
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_URBStateTypeDef  USBD_LL_GetURBState (USBD_HandleTypeDef *phost, uint8_t pipe)
{
  return (USBD_URBStateTypeDef)HAL_HCD_HC_GetURBState (phost->pData, pipe);
}

/**
  * @brief
  * @param
  * @param
  * @retval Status
  */
USBD_StatusTypeDef  USBD_LL_DriverVBUS (USBD_HandleTypeDef *phost, uint8_t state)
{
 /* USER CODE BEGIN 0 */
 /* USER CODE END 0 */
  if(state == 0)
  {
    /* Drive high Charge pump */
    /* USER CODE BEGIN 1 */
    /* ToDo: Add IOE driver control */
    /* USER CODE END 1 */
    if (phost->id == USBD_ID_FS)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    }
    else if (phost->id == USBD_ID_HS)
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
    if (phost->id == USBD_ID_FS)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    }
    else if (phost->id == USBD_ID_HS)
    {
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
    }
  }

  HAL_Delay(200);
  return USBD_OK;
}

/**
  * @brief  USBD_LL_SetToggle
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
USBD_StatusTypeDef   USBD_LL_SetToggle   (USBD_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle)
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

  return USBD_OK;
}

/**
  * @brief  USBD_LL_GetToggle
  *         Initialize the HOST portion of the driver.
  * @param  phost: Selected device
  * @param  base_address: OTG base address
  * @retval Status
  */
uint8_t  USBD_LL_GetToggle   (USBD_HandleTypeDef *phost, uint8_t pipe)
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
void  USBD_Delay (uint32_t Delay)
{
    HAL_Delay(Delay);
}

/******************************************************************************/
/**
  * @brief  The function is a callback about HID Data events
  * @param  phost: Selected device
  * @retval None
  */
void USBD_HID_EventCallback(USBD_HandleTypeDef *phost)
{
//  osSemaphoreRelease(MenuEvent);
    OS_TRACE(D_DEBUG, "%c", 'x');
}

/******************************************************************************/
/*
 * user callbañk definition
*/
void USBD_UserProcess(USBD_HandleTypeDef *usbd_hd_p, uint8_t id)
{
const OS_Signal signal = OS_ISR_SIGNAL_CREATE(DRV_ID_USBD, OS_SIG_DRV, id);
    if (1 == OS_ISR_SIGNAL_SEND(usbdd_stdin_qhd, signal, OS_MSG_PRIO_HIGH)) {
        OS_ContextSwitchForce();
    }
}

// USBD IRQ handlers----------------------------------------------------------
/******************************************************************************/
/**
* @brief This function handles USB On The Go HS global interrupt.
*/
void OTG_HS_IRQHandler(void);
void OTG_HS_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(OTG_HS_IRQn);
    HAL_HCD_IRQHandler(&hcd_otg_hd);
}

/******************************************************************************/
/**
* @brief This function handles USB On The Go FS global interrupt.
*/
void OTG_FS_IRQHandler(void);
void OTG_FS_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(OTG_FS_IRQn);
    HAL_HCD_IRQHandler(&hcd_otg_hd);
}
