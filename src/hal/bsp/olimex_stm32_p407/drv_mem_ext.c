/**************************************************************************//**
* @file    drv_mem_ext.c
* @brief   External memory driver.
* @author  Aleksandar Mitev
******************************************************************************/
#include <string.h>
#include "stm32f4xx_fsmc.h"
#include "hal.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_mem_ext"

//-----------------------------------------------------------------------------
/// @brief   External memory initialization.
/// @return  #Status.
Status MEM_EXT_Init_(void);

/// @brief   External memory initialization.
/// @return  #Status.
static Status MEM_EXT_SRAM512K_Init(void);

/// @brief   External memory deinitialization.
/// @return  #Status.
static Status MEM_EXT_SRAM512K_DeInit(void);

//-----------------------------------------------------------------------------
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
    memset(drv_mem_ext_v, 0x0, sizeof(drv_mem_ext_v));
    drv_mem_ext_v[DRV_ID_MEM_EXT_SRAM512K] = &drv_mem_ext_sram512k;
    return S_OK;
}

/*****************************************************************************/
Status MEM_EXT_SRAM512K_Init(void)
{
FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
FSMC_NORSRAMTimingInitTypeDef p;
GPIO_InitTypeDef GPIO_InitStructure;
Status s = S_OK;

    D_LOG(D_INFO, "Init: ");
	/* Enable the FSMC Clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);

	/*-- GPIO Configuration ------------------------------------------------------*/
	/* SRAM Data lines configuration */
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
									GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
									GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
									GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

	/* SRAM Address lines configuration */
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
									GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 |
									GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
									GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);

	/* NOE and NWE configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);

	/* NE1 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);

	/* NBL0, NBL1 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_FSMC);

	/*-- FSMC Configuration ------------------------------------------------------*/
	p.FSMC_AddressSetupTime = 0;
	p.FSMC_AddressHoldTime = 0;
	p.FSMC_DataSetupTime = 4;
	p.FSMC_BusTurnAroundDuration = 1;
	p.FSMC_CLKDivision = 0;
	p.FSMC_DataLatency = 0;
	p.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1 , ENABLE);

    D_TRACE_S(D_INFO, s);
	//Memory test
    D_LOG(D_INFO, "Test: ");
    IF_STATUS(s = MEM_EXT_Test()) {
        D_TRACE(D_INFO, "Failed!");
        return s;
    }
    D_TRACE(D_INFO, "Passed");
	memset((void*)MEM_EXT_SRAM_BASE_ADDRESS, 0x00, MEM_EXT_SRAM_SIZE);
    return S_OK;
}

/*****************************************************************************/
Status MEM_EXT_SRAM512K_DeInit(void)
{
	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM1);
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, DISABLE);
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
	pData = (U32*)MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < (MEM_EXT_SRAM_SIZE / sizeof(U32)); i++) {
        *pData++ = i;
	}

	pData = (U32*)MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < (MEM_EXT_SRAM_SIZE / sizeof(U32)); i++) {
		if( *pData++ != i) {
			return S_HARDWARE_FAULT;
		}
	}

	// Data bus test
	pData = (U32*)MEM_EXT_SRAM_BASE_ADDRESS;
	for (i = 0; i < TEST_NUMB; ++i) {
		*pData = RamTest[i];
		if (*pData != RamTest[i]) {
			return S_HARDWARE_FAULT;
		}
	}

	return S_OK;
}

