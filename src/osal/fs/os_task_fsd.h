/***************************************************************************//**
* @file    task_fsd.h
* @brief   File system daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_FSD_H_
#define _OS_TASK_FSD_H_

#include "os_common.h"
#include "os_signal.h"
#include "os_usb.h"

#if (1 == OS_FILE_SYSTEM_ENABLED)
#define OS_DAEMON_NAME_FSD          "FileSysD"
#define OS_SIG_FSD_READY            OS_SIG_USB_LAST
#endif //(1 == OS_FILE_SYSTEM_ENABLED)

#endif // _OS_TASK_FSD_H_