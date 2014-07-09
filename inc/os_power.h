/***************************************************************************//**
* @file    os_power.h
* @brief   OS Power.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_POWER_H_
#define _OS_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"

/**
* \defgroup OS_Power OS_Power
* @{
*/
//------------------------------------------------------------------------------
//To speedup call from IDLE task.
#if defined(CM4F)
#define OS_POWER_STATE_SLEEP()          __WFI()
#endif // CM4F

typedef HAL_PowerState OS_PowerState;
typedef HAL_PowerPrio  OS_PowerPrio;

enum {
    OS_PWR_PRIO_UNDEF,
    OS_PWR_PRIO_DEFAULT = 1,
    OS_PWR_PRIO_MAX     = 255,
    OS_PWR_PRIO_LAST    = OS_PWR_PRIO_MAX
};

//------------------------------------------------------------------------------
/// @brief      Init power.
/// @return     #Status.
Status          OS_PowerInit(void);

/// @brief      Get current system power state.
/// @return     Power state.
OS_PowerState   OS_PowerStateGet(void);

/// @brief      Set current system power state.
/// @param[in]  state           Power state.
/// @return     #Status.
Status          OS_PowerStateSet(const OS_PowerState state);

/// @brief      Get name of the current system power state.
/// @param[in]  state           Power state.
/// @return     Power state name.
ConstStrPtr     OS_PowerStateNameGet(const OS_PowerState state);

/**
* \addtogroup OS_ISR_Power ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Set current system power state.
/// @param[in]  state           Power state.
/// @return     #Status.
Status          OS_ISR_PowerStateSet(const OS_PowerState state);

/**@}*/ //OS_ISR_Power

/**@}*/ //OS_Power

#ifdef __cplusplus
}
#endif

#endif // _OS_POWER_H_
