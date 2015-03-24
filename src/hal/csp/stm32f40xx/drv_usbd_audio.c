/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    13-June-2014
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include "os_config.h"

#if (USBD_ENABLED) && (USBD_AUDIO_ENABLED)

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "os_common.h"
#include "os_supervise.h"
#include "os_debug.h"
#include "os_audio.h"
#include "os_mailbox.h"
#include "osal.h"
#include "os_task_usb.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */

static int8_t  Init         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
static int8_t  DeInit       (uint32_t options);
static int8_t  AudioCmd     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t  VolumeCtl    (uint8_t vol);
static int8_t  MuteCtl      (uint8_t cmd);
static int8_t  PeriodicTC   (uint8_t cmd);
static int8_t  GetState     (void);

USBD_AUDIO_ItfTypeDef usbd_hs_audio_itf =
{
  Init,
  DeInit,
  AudioCmd,
  VolumeCtl,
  MuteCtl,
  PeriodicTC,
  GetState,
};

static OS_QueueHd usbd_qhd; //Single instance!

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Init
  *         Initializes the AUDIO media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Init(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  /*
     Add your initialization code here
  */
const OS_UsbAudioInitArgs init_args = {
    .sample_rate= AudioFreq,
    .volume     = Volume
};
OS_Message* msg_p = OS_ISR_MessageCreate((OS_MessageSrc)DRV_ID_USBD, OS_MSG_USB_AUDIO_INIT, sizeof(init_args), &init_args);
const OS_TaskHd usbd_thd = OS_TaskByNameGet(OS_DAEMON_NAME_USB);
Status s = S_UNDEF;
U8 res = USBD_FAIL;

    if (OS_NULL == usbd_thd) {
        return (res);
    }
    usbd_qhd = OS_TaskStdInGet(usbd_thd);
    if (OS_NULL == usbd_qhd) {
        return (res);
    }
    IF_STATUS(s = OS_ISR_MessageSend(usbd_qhd, msg_p, OS_MSG_PRIO_NORMAL)) {
        if (1 == s) {
            OS_ISR_ContextSwitchForce(s);
            res = USBD_OK;
        }
    } else {
        res = USBD_OK;
    }
    return (res);
}

/**
  * @brief  DeInit
  *         DeInitializes the AUDIO media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t DeInit(uint32_t options)
{
  /*
     Add your deinitialization code here
  */
OS_Message* msg_p = OS_ISR_MessageCreate((OS_MessageSrc)DRV_ID_USBD, OS_MSG_USB_AUDIO_DEINIT, 0, OS_NULL);
Status s = S_UNDEF;
U8 res = USBD_FAIL;
    IF_STATUS(s = OS_ISR_MessageSend(usbd_qhd, msg_p, OS_MSG_PRIO_NORMAL)) {
        if (1 == s) {
            OS_ISR_ContextSwitchForce(s);
            res = USBD_OK;
        }
    } else {
        res = USBD_OK;
    }
    usbd_qhd = OS_NULL;
    return (res);
}


/**
  * @brief  AudioCmd
  *         AUDIO command handler
  * @param  Buf: Buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: command opcode
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AudioCmd (uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
static OS_StorageItemLight stor_item;
Status s = S_UNDEF;
U8 res = USBD_FAIL;

    stor_item.data_p = pbuf;
    stor_item.size   = size;
    switch (cmd) {
        case AUDIO_CMD_PLAY:
            {
            OS_Message* msg_p = OS_ISR_MessageCreate((OS_MessageSrc)DRV_ID_USBD, OS_MSG_USB_AUDIO_PLAY,
                                                     sizeof(stor_item), &stor_item);
                if (OS_NULL != msg_p) {
                    IF_STATUS(s = OS_ISR_MessageSend(usbd_qhd, msg_p, OS_MSG_PRIO_NORMAL)) {
                        if (1 == s) {
                            OS_ISR_ContextSwitchForce(s);
                            res = USBD_OK;
                        }
                    } else {
                        res = USBD_OK;
                    }
                }
            }
            break;
        case AUDIO_CMD_START:
            {
            OS_Message* msg_p = OS_ISR_MessageCreate((OS_MessageSrc)DRV_ID_USBD, OS_MSG_USB_AUDIO_START,
                                                     sizeof(stor_item), &stor_item);
                if (OS_NULL != msg_p) {
                    IF_STATUS(s = OS_ISR_MessageSend(usbd_qhd, msg_p, OS_MSG_PRIO_NORMAL)) {
                        if (1 == s) {
                            OS_ISR_ContextSwitchForce(s);
                            res = USBD_OK;
                        }
                    } else {
                        res = USBD_OK;
                    }
                }
            }
            break;
        default:
            OS_ASSERT(OS_FALSE);
            break;
    }
    return (res);
}

/**
  * @brief  VolumeCtl
  * @param  vol: volume level (0..100)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VolumeCtl (uint8_t vol)
{
const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBD, OS_SIG_USB_AUDIO_VOLUME_SET, vol);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbd_qhd, signal, OS_MSG_PRIO_NORMAL));
    return (USBD_OK);
}

/**
  * @brief  MuteCtl
  * @param  cmd: vmute command
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MuteCtl (uint8_t cmd)
{
const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USBD, OS_SIG_USB_AUDIO_MUTE_SET, cmd);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(usbd_qhd, signal, OS_MSG_PRIO_NORMAL));
    return (USBD_OK);
}

/**
  * @brief  PeriodicTC
  * @param  cmd
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t PeriodicTC (uint8_t cmd)
{
    return (USBD_OK);
}

/**
  * @brief  GetState
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t GetState (void)
{
    return (USBD_OK);
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

#endif //(USBD_ENABLED) && (USBD_AUDIO_ENABLED)
