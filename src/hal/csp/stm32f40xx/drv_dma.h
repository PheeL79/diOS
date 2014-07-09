/**************************************************************************//**
* @file    drv_dma.h
* @brief   DMA driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_DMA_H_
#define _DRV_DMA_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_DMA1,
    DRV_ID_DMA2,
    DRV_ID_DMA_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_dma_v[];

//-----------------------------------------------------------------------------
void DMA_MemCpy8(void* dst_p, const void* src_p, SIZE size8);
void DMA_MemCpy8Async(void* dst_p, const void* src_p, SIZE size8);
void DMA_MemCpy8IsDoneWait(void);

void DMA_MemCpy32(void* dst_p, const void* src_p, SIZE size32);
void DMA_MemCpy32Async(void* dst_p, const void* src_p, SIZE size32);
void DMA_MemCpy32IsDoneWait(void);

#endif // _DRV_DMA_H_