/***************************************************************************//**
* @file    os_mutex.c
* @brief   OS Mutex.
* @author  A. Filyanov
*******************************************************************************/
#include "os_common.h"
#include "os_mutex.h"

/******************************************************************************/
OS_MutexHd OS_MutexCreate(void)
{
    return xSemaphoreCreateMutex();
}

/******************************************************************************/
OS_MutexHd OS_MutexRecursiveCreate(void)
{
    return xSemaphoreCreateRecursiveMutex();
}

/******************************************************************************/
void OS_MutexDelete(const OS_MutexHd mhd)
{
    OS_SemaphoreDelete(mhd);
}

/******************************************************************************/
Status OS_MutexLock(const OS_MutexHd mhd, const OS_TimeMs timeout)
{
    return OS_SemaphoreLock(mhd, timeout);
}

/******************************************************************************/
Status OS_MutexRecursiveLock(const OS_MutexHd mhd, const OS_TimeMs timeout)
{
    return OS_SemaphoreRecursiveLock(mhd, timeout);
}

/******************************************************************************/
Status OS_MutexUnlock(const OS_MutexHd mhd)
{
    return OS_SemaphoreUnlock(mhd);
}

/******************************************************************************/
Status OS_MutexRecursiveUnlock(const OS_MutexHd mhd)
{
    return OS_SemaphoreRecursiveUnlock(mhd);
}

/******************************************************************************/
OS_MutexState OS_MutexTest(const OS_MutexHd mhd)
{
    return OS_SemaphoreTest(mhd);
}

/******************************************************************************/
OS_MutexState OS_MutexRecursiveTest(const OS_MutexHd mhd)
{
    return OS_SemaphoreRecursiveTest(mhd);
}

/******************************************************************************/
OS_TaskHd OS_MutexParentGet(const OS_MutexHd mhd)
{
    return xSemaphoreGetMutexHolder(mhd);
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_MutexLock(const OS_MutexHd mhd)
{
    return OS_ISR_SemaphoreLock(mhd);
}

/******************************************************************************/
Status OS_ISR_MutexUnlock(const OS_MutexHd mhd)
{
    return OS_ISR_SemaphoreUnlock(mhd);
}

/******************************************************************************/
OS_MutexState OS_ISR_MutexTest(const OS_MutexHd mhd)
{
    return OS_ISR_SemaphoreTest(mhd);
}
