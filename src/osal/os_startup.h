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
/// @brief      System tasks startup.
/// @return     #Status.
Status OS_StartupSystem(void);

/// @brief      Application tasks startup.
/// @return     #Status.
Status OS_StartupApplication(void);

/**@}*/ //OS_Startup

#endif // _OS_STARTUP_H_