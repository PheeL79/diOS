/***************************************************************************//**
* @file    osal.h
* @brief   OSAL.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OSAL_H_
#define _OSAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"
#include "os_mutex.h"
#include "os_debug.h"
#include "os_driver.h"
#include "os_power.h"

/**
* \defgroup OSAL OSAL
* @{
*/
//------------------------------------------------------------------------------
#define OS_STORAGE_ITEM_OWNERS_MAX  U8_MAX

typedef struct {
    volatile HAL_Env*   hal_env_p;
    OS_DriverHd         drv_stdin;
    OS_DriverHd         drv_stdout;
    OS_DriverHd         drv_rtc;
} OS_Env;

typedef struct {
    OS_MutexHd  mutex;
    void*       data_p;
    U16         size;
    U8          owners;
} OS_StorageItem;

//------------------------------------------------------------------------------
/// @brief      Init OSAL.
/// @return     #Status.
Status          OSAL_Init(void);

/// @brief      Create a storage item.
/// @param[in]  data_p          Item's data.
/// @param[in]  size            Items's data size.
/// @param[out] item_pp         Item.
/// @return     #Status.
Status          OS_StorageItemCreate(const void* data_p, const U16 size, OS_StorageItem** item_pp);

/// @brief      Delete the storage item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_StorageItemDelete(OS_StorageItem* item_p);

/// @brief      Add storage item owner.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_StorageItemOwnerAdd(OS_StorageItem* item_p);

/// @brief      Lock storage item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_StorageItemLock(OS_StorageItem* item_p, const TimeMs timeout);

/// @brief      Unlock storage item.
/// @param[in]  item_p          Item.
/// @return     #Status.
Status          OS_StorageItemUnlock(OS_StorageItem* item_p);

/**@}*/ //OSAL

#ifdef __cplusplus
}
#endif

#endif // _OSAL_H_