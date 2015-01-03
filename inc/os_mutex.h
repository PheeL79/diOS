/***************************************************************************//**
* @file    os_mutex.h
* @brief   OS Mutex.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_MUTEX_H_
#define _OS_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_task.h"
#include "os_semaphore.h"

/**
* \defgroup OS_Mutex OS_Mutex
* @{
*/
//------------------------------------------------------------------------------
typedef OS_SemaphoreHd OS_MutexHd;
typedef OS_SemaphoreState OS_MutexState;

//------------------------------------------------------------------------------
/// @brief      Create a mutex.
/// @return     Mutex handle.
OS_MutexHd      OS_MutexCreate(void);

/// @brief      Create a recursive mutex.
/// @return     Mutex handle.
OS_MutexHd      OS_MutexRecursiveCreate(void);

/// @brief      Delete the mutex.
/// @param[in]  mhd             Mutex handle.
/// @return     None.
void            OS_MutexDelete(const OS_MutexHd mhd);

/// @brief      Lock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @param[in]  timeout         Mutex locking timeout.
/// @return     #Status.
Status          OS_MutexLock(const OS_MutexHd mhd, const OS_TimeMs timeout);

/// @brief      Recursive lock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @param[in]  timeout         Mutex locking timeout.
/// @return     #Status.
Status          OS_MutexRecursiveLock(const OS_MutexHd mhd, const OS_TimeMs timeout);

/// @brief      Unlock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @return     #Status.
Status          OS_MutexUnlock(const OS_MutexHd mhd);

/// @brief      Recursive unlock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @return     #Status.
Status          OS_MutexRecursiveUnlock(const OS_MutexHd mhd);

/// @brief      Test mutex state.
/// @param[in]  mhd             Mutex handle.
/// @return     Mutex state.
OS_MutexState   OS_MutexTest(const OS_MutexHd mhd);

/// @brief      Test recursive mutex state.
/// @param[in]  mhd             Mutex handle.
/// @return     Mutex state.
OS_MutexState   OS_MutexRecursiveTest(const OS_MutexHd mhd);

/// @brief      Get mutex parent.
/// @param[in]  mhd             Mutex handle.
/// @return     Task handle.
OS_TaskHd       OS_MutexParentGet(const OS_MutexHd mhd);

/**
* \addtogroup OS_ISR_Mutex ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Lock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @return     #Status.
Status          OS_ISR_MutexLock(const OS_MutexHd mhd);

/// @brief      Unlock the mutex.
/// @param[in]  mhd             Mutex handle.
/// @return     #Status.
Status          OS_ISR_MutexUnlock(const OS_MutexHd mhd);

/// @brief      Test mutex state.
/// @param[in]  mhd             Mutex handle.
/// @return     Mutex state.
OS_MutexState   OS_ISR_MutexTest(const OS_MutexHd mhd);

/**@}*/ //OS_ISR_Mutex

/**@}*/ //OS_Mutex

#ifdef __cplusplus
}
#endif

#endif // _OS_MUTEX_H_
