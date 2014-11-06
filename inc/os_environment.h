/***************************************************************************//**
* @file    os_environment.h
* @brief   OS Environment.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_ENVIRONMENT_H_
#define _OS_ENVIRONMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_debug.h"
#include "os_task.h"
#include "os_signal.h"
#include "os_driver.h"
#include "os_audio.h"

/**
* \defgroup OS_Environment OS_Environment
* @{
*/

//------------------------------------------------------------------------------
typedef Status (*OS_EnvVariableHandler)(ConstStrPtr variable_value_p);

//------------------------------------------------------------------------------
// System environment variables handlers.

/// @brief      Get system input driver.
/// @return     Driver handle.
OS_DriverHd     OS_DriverStdInGet(void);

/// @brief      Get system output driver.
/// @return     Driver handle.
OS_DriverHd     OS_DriverStdOutGet(void);

/// @brief      Get system realtime clock driver.
/// @return     Driver handle.
OS_DriverHd     OS_DriverRtcGet(void);

/// @brief      Get current system locale.
/// @return     Locale.
Locale          OS_LocaleGet(void);

/// @brief      Set the current system locale.
/// @param[in]  locale_p        Locale.
/// @return     #Status.
Status          OS_LocaleSet(ConstStrPtr locale_p);

/// @brief      Get system input/output driver interface.
/// @return     Driver interface.
const HAL_DriverItf* OS_StdIoGet(void);

/// @brief      Set system input/output driver.
/// @param[in]  drv_name_p      Driver name.
/// @return     #Status.
Status          OS_StdIoSet(ConstStrPtr drv_name_p);

/// @brief      Get current log level of trace details.
/// @return     Log level.
OS_LogLevel     OS_LogLevelGet(void);

/// @brief      Set current log level of trace details.
/// @param[in]  log_level_p     Log level name.
/// @return     #Status.
Status          OS_LogLevelSet(ConstStrPtr log_level_p);

/// @brief      Get current audio device volume.
/// @return     Volume.
OS_AudioVolume  OS_VolumeGet(void);

/// @brief      Set current audio device volume.
/// @param[in]  volume_p        Volume.
/// @return     #Status.
Status          OS_VolumeSet(ConstStrPtr volume_p);

/**
* \addtogroup OS_EnvironmentUser Environment variables user access functions.
* @{
*/
/// @warning Please, do not use environment variables in time critical parts of code.
///          If it possible cache these values locally.
//------------------------------------------------------------------------------
/// @brief      Get the variable owner.
/// @param[in]  variable_name_p     Variable name.
/// @return     Task handle.
OS_TaskHd       OS_EnvVariableOwnerGet(ConstStrPtr variable_name_p);

/// @brief      Get the variable handler.
/// @param[in]  variable_name_p     Variable name.
/// @return     Handle func.
OS_EnvVariableHandler OS_EnvVariableHandlerGet(ConstStrPtr variable_name_p);

/// @brief      Get the environment variable value.
/// @param[in]  variable_name_p     Variable name.
/// @return     Value.
ConstStrPtr     OS_EnvVariableGet(ConstStrPtr variable_name_p);

/// @brief      Set the environment variable value.
/// @param[in]  variable_name_p     Variable name.
/// @param[in]  variable_value_p    Variable value.
/// @param[in]  variable_handler_p  Variable handler func.
/// @return     #Status.
Status          OS_EnvVariableSet(ConstStrPtr variable_name_p, ConstStrPtr variable_value_p,
                                  const OS_EnvVariableHandler variable_handler_p);

/// @brief      Delete the environment variable.
/// @param[in]  variable_name_p     Variable name.
/// @return     #Status.
Status          OS_EnvVariableDelete(ConstStrPtr variable_name_p);

/// @brief      Get the next environment variable.
/// @param[in]  variable_name_p     Variable name.
/// @return     Variable name.
ConstStrPtr     OS_EnvVariableNextGet(ConstStrPtr variable_name_p);

/**@}*/ //OS_EnvironmentUser

/**@}*/ //OS_Environment

#ifdef __cplusplus
}
#endif

#endif // _OS_ENVIRONMENT_H_
