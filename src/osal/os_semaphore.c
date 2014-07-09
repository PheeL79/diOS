/***************************************************************************//**
* @file    os_semaphore.c
* @brief   OS Semaphore.
* @author  A. Filyanov
*******************************************************************************/
#include "FreeRTOS.h"
#include "semphr.h"
#include "os_common.h"
#include "os_semaphore.h"

/******************************************************************************/
void OS_SemaphoreBinaryCreate(OS_SemaphoreHd shd)
{
    vSemaphoreCreateBinary(shd);
}

/******************************************************************************/
OS_SemaphoreHd OS_SemaphoreCountingCreate(const U32 count_max, const U32 count_init)
{
    return xSemaphoreCreateCounting(count_max, count_init);
}

/******************************************************************************/
void OS_SemaphoreDelete(const OS_SemaphoreHd shd)
{
    vSemaphoreDelete(shd);
}

/******************************************************************************/
Status OS_SemaphoreLock(const OS_SemaphoreHd shd, const TimeMs timeout)
{
    if (pdTRUE != xSemaphoreTake(shd, OS_MS_TO_TICKS(timeout))) {
        return S_TIMEOUT;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_SemaphoreUnlock(const OS_SemaphoreHd shd)
{
    if (pdTRUE != xSemaphoreGive(shd)) {
        return S_OVERFLOW;
    }
    return S_OK;
}

/******************************************************************************/
OS_SemaphoreState OS_SemaphoreCheck(const OS_SemaphoreHd shd)
{
    IF_STATUS(OS_SemaphoreLock(shd, OS_NO_BLOCK)) {
        return LOCKED;
    }
    return UNLOCKED;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
Status OS_ISR_SemaphoreLock(const OS_SemaphoreHd shd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (pdTRUE != xSemaphoreTakeFromISR(shd, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
Status OS_ISR_SemaphoreUnlock(const OS_SemaphoreHd shd)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (pdTRUE != xSemaphoreGiveFromISR(shd, &xHigherPriorityTaskWoken)) {
        return S_OVERFLOW;
    }
    if (xHigherPriorityTaskWoken) {
        return 1;
    }
    return S_OK;
}

/******************************************************************************/
OS_SemaphoreState OS_ISR_SemaphoreCheck(const OS_SemaphoreHd shd)
{
    IF_STATUS(OS_ISR_SemaphoreLock(shd)) {
        return LOCKED;
    }
    return UNLOCKED;
}