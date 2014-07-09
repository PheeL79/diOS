#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "os_driver.h"

//------------------------------------------------------------------------------
int MMC_disk_initialize(OS_DriverHd *drv_sdio_sd_p);
int MMC_disk_status(void);

#endif // _SDCARD_H_