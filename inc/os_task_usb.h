/***************************************************************************//**
* @file    os_task_usb.h
* @brief   USB Host/Device daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_USB_H_
#define _OS_TASK_USB_H_

#include "os_config.h"

#if (HAL_USBH_ENABLED) || (HAL_USBD_ENABLED)
#define OS_DAEMON_NAME_USB          "UsbD"
#endif //(HAL_USBH_ENABLED) || (HAL_USBD_ENABLED)

#endif // _OS_TASK_USB_H_