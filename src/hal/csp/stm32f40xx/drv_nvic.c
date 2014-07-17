/**************************************************************************//**
* @file    drv_nvic.c
* @brief   NVIC driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_nvic"

//-----------------------------------------------------------------------------
/// @brief      Init NVIC.
/// @return     #Status.
Status          NVIC_Init_(void);

static Status   NVIC__Init(void);

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
    memset(drv_nvic_v, 0x0, sizeof(drv_nvic_v));
    drv_nvic_v[DRV_ID_NVIC] = &drv_nvic;
    return S_OK;
}

/*****************************************************************************/
Status NVIC__Init(void)
{
    D_LOG(D_INFO, "Init: ");
    NVIC_SetPriorityGrouping(0); //By FreeRTOS request.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //By FreeRTOS request.
    /* Set Priority for SysTick Interrupts */
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0)); // highest priority
    NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, ENABLE);
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

// Exception handlers ----------------------------------------------------------
/******************************************************************************/
void HardFault_Handler(void);
void HardFault_Handler(void)
{
    D_ASSERT(OS_FALSE);
}