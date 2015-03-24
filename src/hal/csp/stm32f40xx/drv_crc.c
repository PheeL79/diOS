/**************************************************************************//**
* @file    drv_crc.c
* @brief   CRC driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"

#if (HAL_CRC_ENABLED)

//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_crc"

//-----------------------------------------------------------------------------
/// @brief   Init CRC.
/// @return  #Status.
Status CRC_Init_(void);

static Status   CRC__Init(void* args_p);
static Status   CRC__DeInit(void* args_p);

//------------------------------------------------------------------------------
extern HAL_DriverItf drv_crc;

//-----------------------------------------------------------------------------
static CRC_HandleTypeDef crc_hd;
HAL_DriverItf* drv_crc_v[DRV_ID_CRC_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_crc = {
    .Init   = CRC__Init,
    .DeInit = CRC__DeInit,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status CRC_Init_(void)
{
Status s = S_UNDEF;
    OS_MemSet(drv_crc_v, 0x0, sizeof(drv_crc_v));
    drv_crc_v[DRV_ID_CRC] = &drv_crc;
    IF_STATUS(s = drv_crc_v[DRV_ID_CRC]->Init(OS_NULL)) { return s; }
    return s;
}

/******************************************************************************/
Status CRC__Init(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init: ");
    __CRC_CLK_ENABLE();
    crc_hd.Instance = CRC;
    if (HAL_OK == HAL_CRC_Init(&crc_hd)) {
        s = S_OK;
    }
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status CRC__DeInit(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "DeInit: ");
    if (HAL_OK == HAL_CRC_DeInit(&crc_hd)) {
        s = S_OK;
    }
    __CRC_CLK_DISABLE();
    HAL_TRACE_S(D_INFO, s);
    return s;
}

#endif //(HAL_CRC_ENABLED)