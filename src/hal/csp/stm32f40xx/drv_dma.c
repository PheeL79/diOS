/**************************************************************************//**
* @file    drv_dma.c
* @brief   DMA driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_dma"

#define DMA_STREAM_MEM2MEM8         DMA2_Stream7
#define DMA_CHANNEL_MEM2MEM8        DMA_Channel_6
#define DMA_IT_TCIF_MEM2MEM8        DMA_IT_TCIF7
#define DMA_IT_TEIF_MEM2MEM8        DMA_IT_TEIF7

#define DMA_STREAM_MEM2MEM32        DMA2_Stream1
#define DMA_CHANNEL_MEM2MEM32       DMA_Channel_0
#define DMA_IT_TCIF_MEM2MEM32       DMA_IT_TCIF1
#define DMA_IT_TEIF_MEM2MEM32       DMA_IT_TEIF1

//-----------------------------------------------------------------------------
/// @brief      DMA initialize.
/// @return     #Status.
Status          DMA_Init_(void);
static Status   DMA2_Init(void);
static Status   DMA2_MEM_Init(void);
static Status   DMA2_NVIC_Init(void);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_dma_v[DRV_ID_DMA_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_dma2 = {
    .Init   = DMA2_Init,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status DMA_Init_(void)
{
    HAL_MEMSET(drv_dma_v, 0x0, sizeof(drv_dma_v));
    drv_dma_v[DRV_ID_DMA2] = &drv_dma2;
    return S_OK;
}

/*****************************************************************************/
Status DMA2_Init(void)
{
Status s;
    HAL_LOG(D_INFO, "Init");
    IF_STATUS(s = DMA2_MEM_Init())    { return s; }
    IF_STATUS(s = DMA2_NVIC_Init())   { return s; }
    return s;
}

/*****************************************************************************/
Status DMA2_NVIC_Init(void)
{
//NVIC_InitTypeDef NVIC_InitStructure;
//
//    HAL_LOG(D_INFO, "NVIC Init: ");
//    NVIC_StructInit(&NVIC_InitStructure);
//    // MEM_2_MEM_32_DMA2_Stream1 enable the interrupt in the NVIC
//    NVIC_InitStructure.NVIC_IRQChannel                  = DMA2_Stream1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MAX;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//    // MEM_2_MEM_8_DMA2_Stream7 enable the interrupt in the NVIC
//    NVIC_InitStructure.NVIC_IRQChannel                  = DMA2_Stream7_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MAX;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//
//    DMA_ITConfig(DMA2_Stream1, DMA_IT_TE | DMA_IT_TC, ENABLE);
//    DMA_ITConfig(DMA2_Stream7, DMA_IT_TE | DMA_IT_TC, ENABLE);
//    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}


/*****************************************************************************/
Status DMA2_MEM_Init(void)
{
//DMA_InitTypeDef DMA_InitStructure;
//
//    HAL_LOG(D_INFO, "MEM Init: ");
//    DMA_StructInit(&DMA_InitStructure);
//    // Set up the DMA
//    // first enable the clock
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
//    // start with a blank DMA configuration just to be sure
//    DMA_DeInit(DMA_STREAM_MEM2MEM8);
//    DMA_DeInit(DMA_STREAM_MEM2MEM32);
//    // MEM_2_MEM_8_DMA_Channel configuration ---------------------------------
//    DMA_InitStructure.DMA_PeripheralBaseAddr= (U32)OS_NULL;
//    DMA_InitStructure.DMA_Memory0BaseAddr   = (U32)OS_NULL;
//    DMA_InitStructure.DMA_BufferSize        = (U32)1;
//    DMA_InitStructure.DMA_DIR               = DMA_DIR_MemoryToMemory;
//    DMA_InitStructure.DMA_PeripheralInc     = DMA_PeripheralInc_Enable;
//    DMA_InitStructure.DMA_MemoryInc         = DMA_MemoryInc_Enable;
//    DMA_InitStructure.DMA_PeripheralDataSize= DMA_PeripheralDataSize_Byte;
//    DMA_InitStructure.DMA_MemoryDataSize    = DMA_MemoryDataSize_Byte;
//    DMA_InitStructure.DMA_Mode              = DMA_Mode_Normal;
//    DMA_InitStructure.DMA_Priority          = DMA_Priority_High;
//    DMA_InitStructure.DMA_Channel           = DMA_CHANNEL_MEM2MEM8;
//    DMA_InitStructure.DMA_FIFOMode          = DMA_FIFOMode_Disable;
//    DMA_InitStructure.DMA_FIFOThreshold     = DMA_FIFOThreshold_Full;
//    DMA_InitStructure.DMA_MemoryBurst       = DMA_MemoryBurst_Single;
//    DMA_InitStructure.DMA_PeripheralBurst   = DMA_PeripheralBurst_Single;
//    DMA_Init(DMA_STREAM_MEM2MEM8, &DMA_InitStructure);
//    // MEM_2_MEM_32_DMA_Channel configuration ---------------------------------
//    DMA_InitStructure.DMA_PeripheralDataSize= DMA_PeripheralDataSize_Word;
//    DMA_InitStructure.DMA_MemoryDataSize    = DMA_MemoryDataSize_Word;
//    DMA_InitStructure.DMA_Channel           = DMA_CHANNEL_MEM2MEM32;
//    DMA_Init(DMA_STREAM_MEM2MEM32, &DMA_InitStructure);
//    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
#pragma inline
void DMA_MemCpy8IsDoneWait(void)
{
//    while (DISABLE != DMA_GetCmdStatus(DMA_STREAM_MEM2MEM8)) {};
}

/*****************************************************************************/
#pragma inline
void DMA_MemCpy8Async(void* dst_p, const void* src_p, SIZE size8)
{
//    while (DISABLE != DMA_GetCmdStatus(DMA_STREAM_MEM2MEM8)) {};
//    DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM8, DMA_IT_TCIF_MEM2MEM8);
//    DMA_STREAM_MEM2MEM8->PAR    = (U32)src_p;
//    DMA_STREAM_MEM2MEM8->M0AR   = (U32)dst_p;
//    DMA_STREAM_MEM2MEM8->NDTR   = (U32)size8;
//    /* DMA Stream enable */
//    DMA_Cmd(DMA_STREAM_MEM2MEM8, ENABLE);
}

/*****************************************************************************/
void DMA_MemCpy8(void* dst_p, const void* src_p, SIZE size8)
{
    DMA_MemCpy8Async(dst_p, src_p, size8);
    DMA_MemCpy8IsDoneWait();
}

/*****************************************************************************/
#pragma inline
void DMA_MemCpy32IsDoneWait(void)
{
//    while (DISABLE != DMA_GetCmdStatus(DMA_STREAM_MEM2MEM32)) {};
}

/*****************************************************************************/
#pragma inline
void DMA_MemCpy32Async(void* dst_p, const void* src_p, SIZE size32)
{
//    while (DISABLE != DMA_GetCmdStatus(DMA_STREAM_MEM2MEM32)) {};
//    DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM32, DMA_IT_TCIF_MEM2MEM32);
//    DMA_STREAM_MEM2MEM32->PAR   = (U32)src_p;
//    DMA_STREAM_MEM2MEM32->M0AR  = (U32)dst_p;
//    DMA_STREAM_MEM2MEM32->NDTR  = (U32)size32;
//    /* DMA Stream enable */
//    DMA_Cmd(DMA_STREAM_MEM2MEM32, ENABLE);
}

/*****************************************************************************/
void DMA_MemCpy32(void* dst_p, const void* src_p, SIZE size32)
{
    DMA_MemCpy32Async(dst_p, src_p, size32);
    DMA_MemCpy32IsDoneWait();
}

/*****************************************************************************/
// MEM 2 MEM 8 DMA Stream interrupt handler
//void DMA2_Stream7_IRQHandler(void);
//void DMA2_Stream7_IRQHandler(void)
//{
//    /* Test on DMA Stream Transfer Complete interrupt */
//    if (DMA_GetITStatus(DMA_STREAM_MEM2MEM8, DMA_IT_TCIF_MEM2MEM8)) {
//        /* Clear DMA Stream Transfer Complete interrupt pending bit */
//        DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM8, DMA_IT_TCIF_MEM2MEM8);
//    } else if (DMA_GetITStatus(DMA_STREAM_MEM2MEM8, DMA_IT_TEIF_MEM2MEM8)) {
//        DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM8, DMA_IT_TEIF_MEM2MEM8);
//        HAL_LOG(D_WARNING, "DMA2 Str7 IRQ err!");
//    } else {
//        D_ASSERT(OS_FALSE);
//    }
//}

/*****************************************************************************/
// MEM 2 MEM 32 DMA Stream interrupt handler
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void)
{
//    /* Test on DMA Stream Transfer Complete interrupt */
//    if (DMA_GetITStatus(DMA_STREAM_MEM2MEM32, DMA_IT_TCIF_MEM2MEM32)) {
//        /* Clear DMA Stream Transfer Complete interrupt pending bit */
//        DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM32, DMA_IT_TCIF_MEM2MEM32);
//    } else if (DMA_GetITStatus(DMA_STREAM_MEM2MEM32, DMA_IT_TEIF_MEM2MEM32)) {
//        DMA_ClearITPendingBit(DMA_STREAM_MEM2MEM32, DMA_IT_TEIF_MEM2MEM32);
//        HAL_LOG(D_WARNING, "DMA2 Str1 IRQ err!");
//    } else {
//        D_ASSERT(OS_FALSE);
//    }
}