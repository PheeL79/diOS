/**************************************************************************//**
* @file    drv_mem_ext.c
* @brief   External memory driver.
* @author  Aleksandar Mitev
******************************************************************************/
#include <string.h>
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_mem_ext"

//-----------------------------------------------------------------------------
/* #define SRAM_MEMORY_WIDTH    FSMC_NORSRAM_MEM_BUS_WIDTH_8 */
#define SRAM_MEMORY_WIDTH    FSMC_NORSRAM_MEM_BUS_WIDTH_16

#define SRAM_BURSTACCESS    FSMC_BURST_ACCESS_MODE_DISABLE
/* #define SRAM_BURSTACCESS    FSMC_BURST_ACCESS_MODE_ENABLE */

#define SRAM_WRITEBURST    FSMC_WRITE_BURST_DISABLE
/* #define SRAM_WRITEBURST   FSMC_WRITE_BURST_ENABLE */

/* DMA definitions for SRAM DMA transfer */
#define __SRAM_DMAx_CLK_ENABLE            __DMA2_CLK_ENABLE
#define SRAM_DMAx_CHANNEL                 DMA_CHANNEL_0
#define SRAM_DMAx_STREAM                  DMA2_Stream0
#define SRAM_DMAx_IRQn                    DMA2_Stream0_IRQn
#define SRAM_DMAx_IRQHandler              DMA2_Stream0_IRQHandler

//-----------------------------------------------------------------------------
/// @brief   External memory initialization.
/// @return  #Status.
Status MEM_EXT_Init_(void);

/// @brief   External memory initialization.
/// @return  #Status.
static Status MEM_EXT_SRAM512K_Init(void* args_p);

/// @brief   External memory deinitialization.
/// @return  #Status.
static Status MEM_EXT_SRAM512K_DeInit(void* args_p);

//-----------------------------------------------------------------------------
SRAM_HandleTypeDef sram1_hd;
static DMA_HandleTypeDef sram1_dma_hd;

HAL_DriverItf* drv_mem_ext_v[DRV_ID_MEM_EXT_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_mem_ext_sram512k = {
    .Init   = MEM_EXT_SRAM512K_Init,
    .DeInit = MEM_EXT_SRAM512K_DeInit,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

/*****************************************************************************/
Status MEM_EXT_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_mem_ext_v, 0x0, sizeof(drv_mem_ext_v));
    drv_mem_ext_v[DRV_ID_MEM_EXT_SRAM512K] = &drv_mem_ext_sram512k;
    IF_STATUS(s = drv_mem_ext_v[DRV_ID_MEM_EXT_SRAM512K]->Init(OS_NULL)) { return s; }
    return s;
}

/*****************************************************************************/
Status MEM_EXT_SRAM512K_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_OK;
    HAL_LOG(L_INFO, "Init: ");
    /* Peripheral clock enable */
    __FSMC_CLK_ENABLE();
    /* Enable chosen DMAx clock */
    __SRAM_DMAx_CLK_ENABLE();
    /* GPIO Ports Clock Enable */
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    /** FSMC GPIO Configuration
    PF0   ------> FSMC_A0
    PF1   ------> FSMC_A1
    PF2   ------> FSMC_A2
    PF3   ------> FSMC_A3
    PF4   ------> FSMC_A4
    PF5   ------> FSMC_A5
    PF12   ------> FSMC_A6
    PF13   ------> FSMC_A7
    PF14   ------> FSMC_A8
    PF15   ------> FSMC_A9
    PG0   ------> FSMC_A10
    PG1   ------> FSMC_A11
    PE7   ------> FSMC_D4
    PE8   ------> FSMC_D5
    PE9   ------> FSMC_D6
    PE10   ------> FSMC_D7
    PE11   ------> FSMC_D8
    PE12   ------> FSMC_D9
    PE13   ------> FSMC_D10
    PE14   ------> FSMC_D11
    PE15   ------> FSMC_D12
    PD8   ------> FSMC_D13
    PD9   ------> FSMC_D14
    PD10   ------> FSMC_D15
    PD14   ------> FSMC_D0
    PD15   ------> FSMC_D1
    PG2   ------> FSMC_A12
    PG3   ------> FSMC_A13
    PG4   ------> FSMC_A14
    PG5   ------> FSMC_A15
    PD0   ------> FSMC_D2
    PD1   ------> FSMC_D3
    PD4   ------> FSMC_NOE
    PD5   ------> FSMC_NWE
    PD7   ------> FSMC_NE1
    PE0   ------> FSMC_NBL0
    PE1   ------> FSMC_NBL1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure common DMA parameters */
    sram1_dma_hd.Init.Channel             = SRAM_DMAx_CHANNEL;
    sram1_dma_hd.Init.Direction           = DMA_MEMORY_TO_MEMORY;
    sram1_dma_hd.Init.PeriphInc           = DMA_PINC_ENABLE;
    sram1_dma_hd.Init.MemInc              = DMA_MINC_ENABLE;
    sram1_dma_hd.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    sram1_dma_hd.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    sram1_dma_hd.Init.Mode                = DMA_NORMAL;
    sram1_dma_hd.Init.Priority            = DMA_PRIORITY_HIGH;
    sram1_dma_hd.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    sram1_dma_hd.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    sram1_dma_hd.Init.MemBurst            = DMA_MBURST_INC8;
    sram1_dma_hd.Init.PeriphBurst         = DMA_PBURST_INC8;

    sram1_dma_hd.Instance = SRAM_DMAx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&sram1_hd, hdma, sram1_dma_hd);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sram1_dma_hd);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sram1_dma_hd);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SRAM_DMAx_IRQn, HAL_PRIO_IRQ_DMA_SRAM, 0);
    HAL_NVIC_EnableIRQ(SRAM_DMAx_IRQn);

    FSMC_NORSRAM_TimingTypeDef Timing;

    /** Perform the SRAM1 memory initialization sequence
    */
    sram1_hd.Instance                 = FSMC_NORSRAM_DEVICE;
    sram1_hd.Extended                 = FSMC_NORSRAM_EXTENDED_DEVICE;
    /* sram1_hd.Init */
    sram1_hd.Init.NSBank              = FSMC_NORSRAM_BANK1;
    sram1_hd.Init.DataAddressMux      = FSMC_DATA_ADDRESS_MUX_DISABLE;
    sram1_hd.Init.MemoryType          = FSMC_MEMORY_TYPE_SRAM;
    sram1_hd.Init.MemoryDataWidth     = SRAM_MEMORY_WIDTH;
    sram1_hd.Init.BurstAccessMode     = SRAM_BURSTACCESS;
    sram1_hd.Init.WaitSignalPolarity  = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    sram1_hd.Init.WrapMode            = FSMC_WRAP_MODE_DISABLE;
    sram1_hd.Init.WaitSignalActive    = FSMC_WAIT_TIMING_BEFORE_WS;
    sram1_hd.Init.WriteOperation      = FSMC_WRITE_OPERATION_ENABLE;
    sram1_hd.Init.WaitSignal          = FSMC_WAIT_SIGNAL_DISABLE;
    sram1_hd.Init.ExtendedMode        = FSMC_EXTENDED_MODE_DISABLE;
    sram1_hd.Init.AsynchronousWait    = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    sram1_hd.Init.WriteBurst          = SRAM_WRITEBURST;
    /* Timing */
    Timing.AddressSetupTime         = 0;
    Timing.AddressHoldTime          = 0;
    Timing.DataSetupTime            = 4;
    Timing.BusTurnAroundDuration    = 1;
    Timing.CLKDivision              = 0;
    Timing.DataLatency              = 0;
    Timing.AccessMode               = FSMC_ACCESS_MODE_A;
    /* ExtTiming */

    HAL_SRAM_Init(&sram1_hd, &Timing, NULL);

    HAL_TRACE_S(L_INFO, s);
	//Memory test
    HAL_LOG(L_INFO, "Test: ");
    IF_STATUS(s = MEM_EXT_Test()) {
        HAL_TRACE(L_INFO, "Failed!");
        return s;
    }
    HAL_TRACE(L_INFO, "Passed");
	HAL_MemSet((void*)HAL_MEM_EXT_SRAM_BASE_ADDRESS, 0x00, HAL_MEM_EXT_SRAM_SIZE);
    return S_OK;
}

/*****************************************************************************/
Status MEM_EXT_SRAM512K_DeInit(void* args_p)
{
    /* Peripheral clock enable */
    __FSMC_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_7);
    return S_OK;
}

/*****************************************************************************/
Status MEM_EXT_Test(void)
{
#define TEST_NUMB 68
const unsigned long RamTest[TEST_NUMB] = {
    0xFFFFFFFF,
    0xFFFFFFFE,
    0xFFFFFFFC,
    0xFFFFFFF8,
    0xFFFFFFF0,

    0xFFFFFFEF,
    0xFFFFFFCF,
    0xFFFFFF8F,
    0xFFFFFF0F,

    0xFFFFFEFF,
    0xFFFFFCFF,
    0xFFFFF8FF,
    0xFFFFF0FF,

    0xFFFFEFFF,
    0xFFFFCFFF,
    0xFFFF8FFF,
    0xFFFF0FFF,

    0xFFFEFFFF,
    0xFFFCFFFF,
    0xFFF8FFFF,
    0xFFF0FFFF,

    0xFFEFFFFF,
    0xFFCFFFFF,
    0xFF8FFFFF,
    0xFF0FFFFF,

    0xFEFFFFFF,
    0xFCFFFFFF,
    0xF8FFFFFF,
    0xF0FFFFFF,

    0xEFFFFFFF,
    0xCFFFFFFF,
    0x8FFFFFFF,
    0x0FFFFFFF,

    ~0xFFFFFFFF,
    ~0xFFFFFFFE,
    ~0xFFFFFFFC,
    ~0xFFFFFFF8,
    ~0xFFFFFFF0,

    ~0xFFFFFFEF,
    ~0xFFFFFFCF,
    ~0xFFFFFF8F,
    ~0xFFFFFF0F,

    ~0xFFFFFEFF,
    ~0xFFFFFCFF,
    ~0xFFFFF8FF,
    ~0xFFFFF0FF,

    ~0xFFFFEFFF,
    ~0xFFFFCFFF,
    ~0xFFFF8FFF,
    ~0xFFFF0FFF,

    ~0xFFFEFFFF,
    ~0xFFFCFFFF,
    ~0xFFF8FFFF,
    ~0xFFF0FFFF,

    ~0xFFEFFFFF,
    ~0xFFCFFFFF,
    ~0xFF8FFFFF,
    ~0xFF0FFFFF,

    ~0xFEFFFFFF,
    ~0xFCFFFFFF,
    ~0xF8FFFFFF,
    ~0xF0FFFFFF,

    ~0xEFFFFFFF,
    ~0xCFFFFFFF,
    ~0x8FFFFFFF,
    ~0x0FFFFFFF,

    0xAAAAAAAA,
    0x55555555,
};

U32 i;
U32*pData;

	// Address bus test
	pData = (U32*)HAL_MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < (HAL_MEM_EXT_SRAM_SIZE / sizeof(U32)); i++) {
        *pData++ = i;
	}

	pData = (U32*)HAL_MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < (HAL_MEM_EXT_SRAM_SIZE / sizeof(U32)); i++) {
		if( *pData++ != i) {
			return S_HARDWARE_ERROR;
		}
	}

	// Data bus test
	pData = (U32*)HAL_MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < TEST_NUMB; ++i) {
		*pData = RamTest[i];
		if (*pData != RamTest[i]) {
			return S_HARDWARE_ERROR;
		}
	}

	return S_OK;
}

/**
  * @brief  Handles SRAM DMA transfer interrupt request.
  * @param  None
  * @retval None
  */
void SRAM_DMAx_IRQHandler(void);
void SRAM_DMAx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(sram1_hd.hdma);
}

