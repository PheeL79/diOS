/***************************************************************************//**
* @file    os_semaphore.h
* @brief   OS Semaphore.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_SEMAPHORE_H_
#define _OS_SEMAPHORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "semphr.h"
#include "status.h"
#include "os_time.h"

/**
* \defgroup OS_Semaphore OS_Semaphore
* @{
*/
//------------------------------------------------------------------------------
typedef MutexState OS_SemaphoreState;
typedef SemaphoreHandle_t OS_SemaphoreHd;

//------------------------------------------------------------------------------
/// @brief      Create a binary semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     None.
void            OS_SemaphoreBinaryCreate(OS_SemaphoreHd shd);

/// @brief      Create a counting semaphore.
/// @param[in]  count_max       Counter maximum value.
/// @param[in]  count_init      Counter initial value.
/// @return     Semaphore handle.
OS_SemaphoreHd  OS_SemaphoreCountingCreate(const U32 count_max, const U32 count_init);

/// @brief      Delete the semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     None.
void            OS_SemaphoreDelete(const OS_SemaphoreHd shd);

/// @brief      Lock the semaphore.
/// @param[in]  shd             Semaphore handle.
/// @param[in]  timeout         Semaphore locking timeout.
/// @return     #Status.
Status          OS_SemaphoreLock(const OS_SemaphoreHd shd, const OS_TimeMs timeout);

/// @brief      Unlock the semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     #Status.
Status          OS_SemaphoreUnlock(const OS_SemaphoreHd shd);

/// @brief      Lock the recursive semaphore.
/// @param[in]  shd             Semaphore handle.
/// @param[in]  timeout         Semaphore locking timeout.
/// @return     #Status.
Status          OS_SemaphoreRecursiveLock(const OS_SemaphoreHd shd, const OS_TimeMs timeout);

/// @brief      Unlock the recursive semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     #Status.
Status          OS_SemaphoreRecursiveUnlock(const OS_SemaphoreHd shd);

/// @brief      Test semaphore state.
/// @param[in]  shd             semaphore handle.
/// @return     Semaphore state.
OS_SemaphoreState OS_SemaphoreTest(const OS_SemaphoreHd shd);

/// @brief      Test recursive semaphore state.
/// @param[in]  shd             semaphore handle.
/// @return     Semaphore state.
OS_SemaphoreState OS_SemaphoreRecursiveTest(const OS_SemaphoreHd shd);

/**
* \addtogroup OS_ISR_Semaphore ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Lock the semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     #Status.
Status          OS_ISR_SemaphoreLock(const OS_SemaphoreHd shd);

/// @brief      Unlock the semaphore.
/// @param[in]  shd             Semaphore handle.
/// @return     #Status.
Status          OS_ISR_SemaphoreUnlock(const OS_SemaphoreHd shd);

/// @brief      Test semaphore state.
/// @param[in]  shd             semaphore handle.
/// @return     Semaphore state.
OS_SemaphoreState OS_ISR_SemaphoreTest(const OS_SemaphoreHd shd);


/**@}*/ //OS_ISR_Semaphore

/**@}*/ //OS_Semaphore

#ifdef __cplusplus
}
#endif

#endif // _OS_SEMAPHORE_H_
