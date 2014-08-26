/**************************************************************************//**
* @file    drv_usbh.c
* @brief   USB Host driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "usbh_core.h"
#if (1 == USBH_HID_ENABLED)
    #include "usbh_hid.h"
#endif // USBH_HID_ENABLED
#if (1 == USBH_MSC_ENABLED)
    #include "usbh_msc.h"
#endif // USBH_MSC_ENABLED
#include "os_common.h"
#include "os_task.h"
#include "os_supervise.h"
#include "os_message.h"
#include "os_signal.h"
#include "os_debug.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usbh"

//------------------------------------------------------------------------------
/// @brief   USBH initialization.
/// @return  #Status.
Status USBH__Init(void);

static Status   USBH_Init_(void* args_p);
static Status   USBH_DeInit_(void);
static Status   USBH_Open(void* args_p);
static Status   USBH_Close(void);
static Status   USBH_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_IT_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_IT_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USBH_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USBH_IoCtl(const U32 request_id, void* args_p);

static void     USBH_UserProcess(USBH_HandleTypeDef* usbh_hd_p, uint8_t id);

//------------------------------------------------------------------------------
static HCD_HandleTypeDef    hcd_otg_hd;
static USBH_HandleTypeDef*  usbh_hd_p;
OS_QueueHd                  usbhd_stdin_qhd;
HAL_DriverItf*              drv_usbh_v[DRV_ID_USBH_LAST];

static HAL_DriverItf drv_usbh = {
    .Init   = USBH_Init_,
    .DeInit = USBH_DeInit_,
    .Open   = USBH_Open,
    .Close  = USBH_Close,
    .Read   = USBH_Read,
    .Write  = USBH_Write,
    .IoCtl  = USBH_IoCtl
};

/******************************************************************************/
Status USBH__Init(void)
{
Status s = S_OK;
    HAL_MEMSET(drv_usbh_v, 0x0, sizeof(drv_usbh_v));
    drv_usbh_v[DRV_ID_USBH]     = &drv_usbh;
#if (1 == USBH_MSC_ENABLED)
    extern HAL_DriverItf drv_usbh_msc;
    drv_usbh_v[DRV_ID_USBH_MSC] = &drv_usbh_msc;
#endif // USBH_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBH_Init_(void* args_p)
{
Status s = S_OK;
    usbh_hd_p = (USBH_HandleTypeDef*)args_p;
    /* Init Host Library,Add Supported Class and Start the library*/
    if (USBH_OK != USBH_Init(usbh_hd_p, USBH_UserProcess, HOST_FS))  { return s = S_HARDWARE_FAULT; }
#if (1 == USBH_HID_ENABLED)
    if (USBH_OK != USBH_RegisterClass(usbh_hd_p, USBH_HID_CLASS))    { return s = S_HARDWARE_FAULT; }
#endif // USBH_HID_ENABLED
#if (1 == USBH_MSC_ENABLED)
    if (S_OK != drv_usbh_v[DRV_ID_USBH_MSC]->Init(usbh_hd_p))        { return s = S_HARDWARE_FAULT; }
    if (USBH_OK != USBH_RegisterClass(usbh_hd_p, USBH_MSC_CLASS))    { return s = S_HARDWARE_FAULT; }
#endif // USBH_MSC_ENABLED
    return s;
}

/******************************************************************************/
Status USBH_DeInit_(void)
{
Status s = S_OK;
    HAL_HCD_MspDeInit(&hcd_otg_hd);
    if (USBH_OK != USBH_DeInit(usbh_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBH_Open(void* args_p)
{
Status s = S_OK;
    usbhd_stdin_qhd = (OS_QueueHd)args_p;
    if (USBH_OK != USBH_Start(usbh_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBH_Close(void)
{
Status s = S_OK;
    usbhd_stdin_qhd = OS_NULL;
    if (USBH_OK != USBH_Stop(usbh_hd_p)) { return s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USBH_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_IT_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_IT_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status USBH_IoCtl(const U32 request_id, void* args_p)
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
    /* Peripheral clock enable */
    __USB_OTG_FS_CLK_ENABLE();

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
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
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

    /* Peripheral interrupt init*/
    /* Sets the priority grouping field */
//    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    HAL_NVIC_SetPriority(OTG_FS_IRQn, OS_PRIORITY_INT_MAX, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  }
}

void HAL_HCD_MspDeInit(HCD_HandleTypeDef* hhcd)
{
  if(hhcd->Instance==USB_OTG_FS)
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
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
  }
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
#if (USBH_USE_OS == 1)
  USBH_LL_NotifyURBChange(hhcd->pData);
#endif
const OS_Signal signal = OS_ISR_SIGNAL_CREATE(DRV_ID_USBH, OS_SIG_DRV, 0);
    if (1 == OS_ISR_SIGNAL_SEND(usbhd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL)) {
        OS_ContextSwitchForce();
    }
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
  if (phost->id == HOST_FS) {
  /* Link The driver to the stack */
  hcd_otg_hd.pData = phost;
  phost->pData = &hcd_otg_hd;

  hcd_otg_hd.Instance = USB_OTG_FS;
  hcd_otg_hd.Init.Host_channels = 8;
  hcd_otg_hd.Init.speed = HCD_SPEED_FULL;
  hcd_otg_hd.Init.dma_enable = DISABLE;
  hcd_otg_hd.Init.phy_itface = HCD_PHY_EMBEDDED;
  hcd_otg_hd.Init.Sof_enable = DISABLE;
  hcd_otg_hd.Init.low_power_enable = ENABLE;
  hcd_otg_hd.Init.vbus_sensing_enable = ENABLE;
  hcd_otg_hd.Init.use_external_vbus = ENABLE;
  HAL_HCD_Init(&hcd_otg_hd);

  USBH_LL_SetTimer (phost, HAL_HCD_GetCurrentFrame(&hcd_otg_hd));
  }
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
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
  }
  else
  {
    /* Drive low Charge pump */
    /* USER CODE BEGIN 2 */
    /* ToDo: Add IOE driver control */
    /* USER CODE END 2 */
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
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
 * user callba�k definition
*/
void USBH_UserProcess(USBH_HandleTypeDef *usbh_hd_p, uint8_t id)
{
const OS_Signal signal = OS_ISR_SIGNAL_CREATE(DRV_ID_USBH, OS_SIG_DRV, id);
    if (1 == OS_ISR_SIGNAL_SEND(usbhd_stdin_qhd, signal, OS_MSG_PRIO_HIGH)) {
        OS_ContextSwitchForce();
    }
}

// USBH IRQ handlers----------------------------------------------------------
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