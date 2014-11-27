/***************************************************************************//**
* @file    task_usbd.h
* @brief   USB Host/Device daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_USBD_H_
#define _OS_TASK_USBD_H_

#include "os_config.h"

#if (1 == USBH_ENABLED) || (1 == USBD_ENABLED)
#define OS_DAEMON_NAME_USB          "UsbD"
#endif //(1 == USBH_ENABLED) || (1 == USBD_ENABLED)

#endif // _OS_TASK_USBD_H_