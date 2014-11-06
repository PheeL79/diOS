/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    26-June-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hal.h"
#include "stm32f4xx_it.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    HAL_ASSERT(OS_FALSE);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    HAL_ASSERT(OS_FALSE);
  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    HAL_ASSERT(OS_FALSE);
  /* Go to infinite loop when Memory Manage exception occurs */
//  while (1)
//  {
//  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    HAL_ASSERT(OS_FALSE);
  /* Go to infinite loop when Bus Fault exception occurs */
//  while (1)
//  {
//  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    HAL_ASSERT(OS_FALSE);
  /* Go to infinite loop when Usage Fault exception occurs */
//  while (1)
//  {
//  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  * @note   The application need to ensure that the SysTick time base is always set to 1 millisecond
            to have correct HAL operation.
  */
#include "FreeRTOS.h"
#include "task.h"
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (taskSCHEDULER_NOT_STARTED != xTaskGetSchedulerState()) {
        extern void xPortSysTickHandler(void);
        xPortSysTickHandler();
    }
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

#include "os_config.h"
/******************************************************************************/
/**
* @brief This function handles USB On The Go FS global interrupt.
*/
void OTG_FS_IRQHandler(void);
void OTG_FS_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(OTG_FS_IRQn);
#if (1 == USBD_ENABLED)
#if (1 == USBD_FS_ENABLED)
    extern PCD_HandleTypeDef pcd_fs_hd;
    HAL_PCD_IRQHandler(&pcd_fs_hd);
#endif //(1 == USBD_FS_ENABLED)
#endif //(1 == USBD_ENABLED)

#if (1 == USBH_ENABLED)
#if (1 == USBH_FS_ENABLED)
    extern HCD_HandleTypeDef hcd_fs_hd;
    HAL_HCD_IRQHandler(&hcd_fs_hd);
#endif //(1 == USBH_FS_ENABLED)
#endif //(1 == USBH_ENABLED)
}

/******************************************************************************/
/**
* @brief This function handles USB On The Go HS global interrupt.
*/
void OTG_HS_IRQHandler(void);
void OTG_HS_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(OTG_HS_IRQn);
#if (1 == USBD_ENABLED)
#if (1 == USBD_HS_ENABLED)
    extern PCD_HandleTypeDef pcd_hs_hd;
    HAL_PCD_IRQHandler(&pcd_hs_hd);
#endif //(1 == USBD_HS_ENABLED)
#endif //(1 == USBD_ENABLED)

#if (1 == USBH_ENABLED)
#if (1 == USBH_HS_ENABLED)
    extern HCD_HandleTypeDef hcd_hs_hd;
    HAL_HCD_IRQHandler(&hcd_hs_hd);
#endif //(1 == USBH_HS_ENABLED)
#endif //(1 == USBH_ENABLED)
}

/******************************************************************************/
/**
  * @brief  This function handles ADC interrupt request.
*/
void ADC_IRQHandler(void);
void ADC_IRQHandler(void)
{
extern ADC_HandleTypeDef adc3_hd;
    HAL_ADC_IRQHandler(&adc3_hd);
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
