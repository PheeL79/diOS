/***************************************************************************//**
* @file    os_driver.h
* @brief   OS Driver.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_DRIVER_H_
#define _OS_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"
#include "os_task.h"

/**
* \defgroup OS_Driver OS_Driver
* @{
*/
//------------------------------------------------------------------------------
typedef const void* OS_DriverHd;

enum { //Bit fields
    OS_DRV_STATE_UNDEF,
    OS_DRV_STATE_IS_INIT,
    OS_DRV_STATE_IS_OPEN,
    OS_DRV_STATE_LAST = 7
};
typedef U8 OS_DriverState;

typedef struct {
    OS_DriverState      state; //Bit fields.
    OS_PowerState       power;
    U16                 owners;
    U32                 sended;
    U32                 received;
    U32                 errors_cnt;
    Status              status_last;
} OS_DriverStats;

typedef struct {
    ConstStr            name[OS_DRIVER_NAME_LEN];
    HAL_DriverItf*      itf_p;
    OS_PowerPrio        prio_power;
} OS_DriverConfig;

//------------------------------------------------------------------------------
/// @brief      Create driver.
/// @param[in]  cfg_p          Driver's config.
/// @param[out] dhd_p          Driver's handle.
/// @return     #Status.
Status          OS_DriverCreate(const OS_DriverConfig* cfg_p, OS_DriverHd* dhd_p);

/// @brief      Delete driver.
/// @param[in]  dhd            Driver's handle.
/// @return     #Status.
Status          OS_DriverDelete(const OS_DriverHd dhd);

/// @brief      Init driver.
/// @param[in]  dhd            Driver's handle.
/// @return     #Status.
Status          OS_DriverInit(const OS_DriverHd dhd);

/// @brief      Deinit driver.
/// @param[in]  dhd            Driver's handle.
/// @return     #Status.
Status          OS_DriverDeInit(const OS_DriverHd dhd);

/// @brief      Open driver.
/// @param[in]  dhd            Driver's handle.
/// @param[in]  args_p         Driver's input arguments.
/// @return     #Status.
Status          OS_DriverOpen(const OS_DriverHd dhd, void* args_p);

/// @brief      Close driver.
/// @param[in]  dhd            Driver's handle.
/// @return     #Status.
Status          OS_DriverClose(const OS_DriverHd dhd);

/// @brief      Read data.
/// @param[in]  dhd            Driver's handle.
/// @param[out] data_in_p      Data input buffer.
/// @param[in]  size           Data input buffer size.
/// @param[in]  args_p         Driver's specific input arguments (if presents).
/// @return     #Status.
Status          OS_DriverRead(const OS_DriverHd dhd, void* data_in_p, U32 size, void* args_p);

/// @brief      Write data.
/// @param[in]  dhd            Driver's handle.
/// @param[in]  data_out_p     Data output buffer.
/// @param[in]  size           Data output buffer size.
/// @param[in]  args_p         Driver's specific input arguments (if presents).
/// @return     #Status.
Status          OS_DriverWrite(const OS_DriverHd dhd, void* data_out_p, U32 size, void* args_p);

/// @brief      Input/Output control.
/// @param[in]  dhd            Driver's handle.
/// @param[in]  request_id     Driver's request code indentifier.
/// @param[in]  args_p         Driver's specific input arguments (if presents).
/// @return     #Status.
Status          OS_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p);

/// @brief      Get driver name.
/// @param[in]  dhd            Driver's handle.
/// @return     Name.
ConstStrPtr     OS_DriverNameGet(const OS_DriverHd dhd);

/// @brief      Get driver's state name.
/// @param[in]  state          Driver's state.
/// @return     State name.
ConstStrPtr     OS_DriverStateNameGet(const OS_DriverState state);

/// @brief      Get driver by it's name.
/// @param[in]  name_p         Driver's name.
/// @return     Driver handle.
OS_DriverHd     OS_DriverByNameGet(ConstStrPtr name_p);

/// @brief      Get the next driver.
/// @param[in]  dhd            Driver's handle.
/// @return     Driver handle.
OS_DriverHd     OS_DriverNextGet(const OS_DriverHd dhd);

/// @brief      Get driver state.
/// @param[in]  dhd            Driver's handle.
/// @return     Driver state.
OS_DriverState  OS_DriverStateStateGet(const OS_DriverHd dhd);

/// @brief      Get driver stats.
/// @param[in]  dhd            Driver's handle.
/// @param[out] stats_p        Stats.
/// @return     #Status.
Status          OS_DriverStatsGet(const OS_DriverHd dhd, OS_DriverStats* stats_p);

/// @brief      Get driver configuration.
/// @param[in]  dhd            Driver's handle.
/// @return     Driver configuration.
const OS_DriverConfig* OS_DriverConfigGet(const OS_DriverHd dhd);

/// @brief      Get driver's parent.
/// @param[in]  dhd            Driver's handle.
/// @return     Task handle.
OS_TaskHd       OS_DriverParentGet(const OS_DriverHd dhd);

//U32             OS_DriversCountGet(void);

/**
* \addtogroup OS_ISR_Driver ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Input/Output control.
/// @param[in]  dhd            Driver's handle.
/// @param[in]  request_id     Driver's request code indentifier.
/// @param[in]  args_p         Driver's specific input arguments (if presents).
/// @return     #Status.
Status          OS_ISR_DriverIoCtl(const OS_DriverHd dhd, const U32 request_id, void* args_p);

/**@}*/ //OS_ISR_Driver

/**@}*/ //OS_Driver

#ifdef __cplusplus
}
#endif

#endif // _OS_DRIVER_H_
