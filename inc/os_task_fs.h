/***************************************************************************//**
* @file    os_task_fs.h
* @brief   File system daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_FS_H_
#define _OS_TASK_FS_H_

#include "drv_usb.h"
#include "os_common.h"
#include "os_signal.h"

#if (OS_FILE_SYSTEM_ENABLED)
#define OS_DAEMON_NAME_FS           "FileSysD"
#define OS_SIG_FSD_READY            OS_SIG_USB_LAST
#endif //(OS_FILE_SYSTEM_ENABLED)

#endif // _OS_TASK_FS_H_