/**************************************************************************//**
* @file    drv_nvic.c
* @brief   NVIC driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_config.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_nvic"

//-----------------------------------------------------------------------------
/// @brief      Init NVIC.
/// @return     #Status.
Status          NVIC_Init_(void);

static Status   NVIC__Init(void* args_p);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_nvic_v[DRV_ID_NVIC_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_nvic = {
    .Init   = NVIC__Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status NVIC_Init_(void)
{
    HAL_MemSet(drv_nvic_v, 0x0, sizeof(drv_nvic_v));
    drv_nvic_v[DRV_ID_NVIC] = &drv_nvic;
    return S_OK;
}

/*****************************************************************************/
Status NVIC__Init(void* args_p)
{
    HAL_LOG(D_INFO, "Init: ");
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); //By FreeRTOS request.
    /* Set Priority for SysTick Interrupts */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0); // highest priority
    //HAL_NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, ENABLE);
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}
