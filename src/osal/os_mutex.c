/***************************************************************************//**
* @file    os_mutex.c
* @brief   OS Mutex.
* @author  A. Filyanov
*******************************************************************************/
#include "FreeRTOS.h"
#include "semphr.h"
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
Status OS_MutexLock(const OS_MutexHd mhd, const TimeMs timeout)
{
    return OS_SemaphoreLock(mhd, timeout);
}

/******************************************************************************/
Status OS_MutexRecursiveLock(const OS_MutexHd mhd, const TimeMs timeout)
{
    if (pdTRUE != xSemaphoreTakeRecursive(mhd, timeout)) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_MutexUnlock(const OS_MutexHd mhd)
{
    return OS_SemaphoreUnlock(mhd);
}

/******************************************************************************/
Status OS_MutexRecursiveUnlock(const OS_MutexHd mhd)
{
    if (pdTRUE != xSemaphoreGiveRecursive(mhd)) {
        return S_OVERFLOW;
    }
    return S_OK;
}

/******************************************************************************/
OS_MutexState OS_MutexCheck(const OS_MutexHd mhd)
{
    return OS_SemaphoreCheck(mhd);
}

/******************************************************************************/
OS_MutexState OS_MutexRecursiveCheck(const OS_MutexHd mhd)
{
    IF_STATUS(OS_MutexRecursiveLock(mhd, OS_NO_BLOCK)) {
        return LOCKED;
    }
    return UNLOCKED;
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
OS_MutexState OS_ISR_MutexCheck(const OS_MutexHd mhd)
{
    return OS_ISR_SemaphoreCheck(mhd);
}