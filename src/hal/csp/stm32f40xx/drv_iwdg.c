/**************************************************************************//**
* @file    drv_iwdg.c
* @brief   IWDG driver.
* @author
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_iwdg"

//------------------------------------------------------------------------------
/// @brief   IWDG initialize.
/// @return  #Status.
Status IWDG_Init_(void);

static Status   IWDG_Init(void);
static Status   IWDG_DeInit(void);
static Status   IWDG_Open(void* args_p);
static Status   IWDG_Close(void);
static Status   IWDG_Read(U8* data_in_p, U32 size, void* args_p);
static Status   IWDG_Write(U8* data_out_p, U32 size, void* args_p);
static Status   IWDG_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
IWDG_HandleTypeDef iwdg_handle;
HAL_DriverItf* drv_iwdg_v[DRV_ID_IWDG_LAST];

//------------------------------------------------------------------------------
static HAL_DriverItf drv_iwdg = {
    .Init   = IWDG_Init,
    .DeInit = IWDG_DeInit,
    .Open   = IWDG_Open,
    .Close  = IWDG_Close,
    .Read   = IWDG_Read,
    .Write  = IWDG_Write,
    .IoCtl  = IWDG_IoCtl
};

/******************************************************************************/
Status IWDG_Init_(void)
{
Status s = S_OK;
    memset(drv_iwdg_v, 0x0, sizeof(drv_iwdg_v));
    drv_iwdg_v[DRV_ID_IWDG] = &drv_iwdg;
    return s;
}

/******************************************************************************/
Status IWDG_Init(void)
{
Status s = S_OK;
    __HAL_RCC_LSI_ENABLE();
    while (OS_TRUE != __HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)) {};
    /*##-3- Configure the IWDG peripheral ######################################*/
    /* Set counter reload value to obtain 250ms IWDG TimeOut.
     IWDG counter clock Frequency = LsiFreq / 32
     Counter Reload Value = 250ms / IWDG counter clock period
                          = 0.25s / (32/LsiFreq)
                          = LsiFreq / (32 * 4)
                          = LsiFreq / 128 */
    iwdg_handle.Instance        = IWDG;

    iwdg_handle.Init.Prescaler  = IWDG_PRESCALER_32;
    iwdg_handle.Init.Reload     = HAL_WATCHDOG_TIMEOUT;

    if (HAL_OK != HAL_IWDG_Init(&iwdg_handle)) {
        s = S_HARDWARE_FAULT;
    }
    return s;
}

/******************************************************************************/
Status IWDG_DeInit(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status IWDG_Open(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_IWDG_Start(&iwdg_handle)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status IWDG_Close(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status IWDG_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status IWDG_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_IWDG_Refresh(&iwdg_handle)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status IWDG_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case PWR_ON:
            break;
        case PWR_OFF:
            break;
        default:
            HAL_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}
