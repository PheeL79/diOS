/**************************************************************************//**
* @file    drv_pppx.c
* @brief   PPPX driver.
* @author  
******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_pppx"

//------------------------------------------------------------------------------
static Status   PPPX_Init(void);
static Status   PPPX_DeInit(void);
static void     PPPX_GPIO_Init(void);
static void     PPPX_NVIC_Init(void);
static void     PPPX_DMA_Init(void);
static Status   PPPX_Open(void* args_p);
static Status   PPPX_Close(void);
static Status   PPPX_Read(U8* data_in_p, U32 size, void* args_p);
static Status   PPPX_Write(U8* data_out_p, U32 size, void* args_p);
static Status   PPPX_IT_Read(U8* data_in_p, U32 size, void* args_p);
static Status   PPPX_IT_Write(U8* data_out_p, U32 size, void* args_p);
static Status   PPPX_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status   PPPX_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status   PPPX_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
static HAL_DriverItf drv_pppx = {
    .Init   = PPPX_Init,
    .DeInit = PPPX_DeInit,
    .Open   = PPPX_Open,
    .Close  = PPPX_Close,
    .Read   = PPPX_Read,
    .Write  = PPPX_Write,
    .IoCtl  = PPPX_IoCtl
};

/******************************************************************************/
Status PPPX_Init(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
void PPPX_GPIO_Init(void)
{
}

/******************************************************************************/
void PPPX_DMA_Init(void)
{
}

/******************************************************************************/
void PPPX_NVIC_Init(void)
{
}

/******************************************************************************/
Status PPPX_DeInit(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_Open(void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_Close(void)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_IT_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_IT_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    return s;
}

/******************************************************************************/
Status PPPX_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        default:
            HAL_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

// PPP IRQ handlers----------------------------------------------------------
