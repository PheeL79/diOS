/***************************************************************************//**
* @file    os_startup.h
* @brief   OS Startup.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_STARTUP_H_
#define _OS_STARTUP_H_

#include "osal.h"

/**
* \defgroup OS_Startup OS_Startup
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Init startup.
/// @return     #Status.
Status          OS_StartupInit(void);

/// @brief      Deinit startup.
/// @return     #Status.
Status          OS_StartupDeInit(void);

/// @brief      System tasks startup.
/// @return     #Status.
Status          OS_StartupSystem(void);

/// @brief      Application tasks startup.
/// @return     #Status.
Status          OS_StartupApplication(void);

Status          OS_StartupTaskAdd(const OS_TaskConfig* task_cfg_p);

/**@}*/ //OS_Startup

#endif // _OS_STARTUP_H_