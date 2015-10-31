/**************************************************************************//**
* @file    hal_olimex_stm32_p407.c
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#include <yfuns.h>
#include <string.h>
#include "hal.h"
#include "version.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "hal"

//-----------------------------------------------------------------------------
volatile HAL_Env hal_env;
int cycles_diff, cycles_last;
U32 SystemCoreClockKHz;             ///< Core frequency (KHz).
U32 SystemCoreClockMHz;             ///< Core frequency (MHz).
static DeviceState device_state;    ///< Device state.

//-----------------------------------------------------------------------------
/// @brief      Init the device description.
/// @return     #Status.
static Status   HAL_DeviceDescriptionInit(void);
static void     SystemClock_Config(void);

/*****************************************************************************/
Status HAL_Init_(void)
{
extern Status USART_Init_(void);
const Version* ver_p = &device_state.description.device_description.version;
Status s = S_OK;
    // Disable interrupts.
    HAL_CRITICAL_SECTION_ENTER(); {
        SystemInit();
        SystemClock_Config();
        SystemCoreClockKHz  = SystemCoreClock / KHZ;
        SystemCoreClockMHz  = SystemCoreClockKHz / KHZ;
        if (HAL_OK != HAL_Init()) { return S_HARDWARE_ERROR; }
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); //By FreeRTOS request.
        /* Set Priority for SysTick Interrupts */
        HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0); // highest priority
        TIMER_DWT_Init(); // HAL_LOG depends on this init.
        // HAL environment init
        hal_env.locale      = LOC_EN;
        hal_env.power       = PWR_ON;
        hal_env.log_level   = HAL_LOG_LEVEL;
        IF_STATUS(s = USART_Init_()) { return s; }
        hal_env.stdio_p = drv_stdio_p;
        HAL_ASSERT(S_OK == hal_env.stdio_p->Init(OS_NULL));
        HAL_ASSERT(S_OK == hal_env.stdio_p->Open(OS_NULL));
        // Init device description.
        IF_STATUS(HAL_DeviceDescriptionInit()) { return S_MODULE; }
        // Init and open STDIO stream.
        HAL_StdIoCls();
        HAL_LOG(L_INFO, "-------------------------------");
        HAL_LOG(L_INFO, "Board name: %s", HAL_MB_NAME);
        HAL_LOG(L_INFO, "Clock core: %u Hz", HAL_SystemCoreClockGet());
        HAL_LOG(L_INFO, "diOS: v%d.%d.%d%s-%s",
                        ver_p->maj,
                        ver_p->min,
                        ver_p->bld,
                        ver_lbl[ver_p->lbl],
                        ver_p->rev);
        HAL_LOG(L_INFO, "Built on: %s, %s", __DATE__, __TIME__);
        HAL_LOG(L_INFO, "-------------------------------");
        HAL_LOG(L_INFO, "HAL init...");
        HAL_LOG(L_INFO, "-------------------------------");
        //TODO(A. Filyanov) HAL_CSP_Init(); HAL_MSP_Init()?; HAL_BSP_Init();
        IF_STATUS(s = HAL_BSP_Init()) { return s; }
        HAL_LOG(L_INFO, "-------------------------------");
        // Close and deinit STDIO stream(for init HAL output).
        // Stream will be open back again in OSAL init.
        HAL_ASSERT(S_OK == hal_env.stdio_p->Close(OS_NULL));
        HAL_ASSERT(S_OK == hal_env.stdio_p->DeInit(OS_NULL));
    // Enable interrupts.
    } HAL_CRITICAL_SECTION_EXIT();
    return s;
}

/*****************************************************************************/
Status HAL_DeviceDescriptionInit(void)
{
    HAL_MemSet((void*)&device_state, 0x0, sizeof(device_state));
    // Static info description.
    device_state.description.device_description.version = version;
    return S_OK;
}

/*****************************************************************************/
Status HAL_DeviceDescriptionGet(DeviceDesc* dev_desc_p, const U16 size)
{
    if (size < sizeof(device_state.description)) {
        return S_MODULE;
    }
    // Dynamic info description.
//    HAL_DevDesc_IdGet();
//    HAL_DevDesc_RevisionGet();
//    HAL_DevDesc_VoltageCoeffsGet();
//    HAL_DevDesc_VoltageGet();
//    HAL_DevDesc_TemperatureGet();
//    DMA_MemCpy8(dev_desc_p, (const void*)&device_state.description, sizeof(device_state.description));
    return S_OK;
}

/*****************************************************************************/
void HAL_DelayMs(U32 delay_ms)
{
    HAL_Delay(delay_ms);
}

/*****************************************************************************/
void HAL_DelayCycles(U32 cycles)
{
   while (0 != --cycles) {};
}

/*****************************************************************************/
U8 HAL_BitCount(U32 value)
{
extern U32 BitCount(U32);
    return (U8)BitCount(value);
}

/*****************************************************************************/
U32 HAL_BitReverse(register U32 value)
{
    return __RBIT(value);
/*
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
*/
}

/*****************************************************************************/
U32 HAL_ByteReverse(register U32 value)
{
    return __REV(value);
}

/*****************************************************************************/
U8 HAL_ByteToBcd2(U8 value)
{
U8 bcdhigh = 0;
    while (value >= 10) {
        ++bcdhigh;
        value -= 10;
    }
    return ((U8)(bcdhigh << 4) | value);
}

/*****************************************************************************/
U8 HAL_Bcd2ToByte(U8 value)
{
const U8 tmp = ((U8)(value & (U8)0xF0) >> (U8)0x4) * 10;
    return (tmp + (value & (U8)0x0F));
}

/*****************************************************************************/
void HAL_SoftReset(void)
{
    NVIC_SystemReset();
}

/*****************************************************************************/
U32 HAL_SystemCoreClockGet(void)
{
    return SystemCoreClock;
}

/******************************************************************************/
void HAL_StdIoCls(void)
{
    for (register int i = 0; i < HAL_STDIO_TERM_HEIGHT; ++i) {
        if (EOF == putchar('\n')) { break; }
    }
}

/******************************************************************************/
int putchar(int c)
{
    IF_STATUS(hal_env.stdio_p->Write((U8*)&c, 1, OS_NULL)) { return EOF; }
    return c;
}

/******************************************************************************/
int getchar(void)
{
U8 c;
    IF_STATUS(hal_env.stdio_p->Read((U8*)&c, 1, OS_NULL)) { return EOF; }
    return c;
}

/******************************************************************************/
void SystemClock_Config(void)
{
RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();
    /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(HAL_PWR_REGULATOR_VOLTAGE_SCALE);
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType= HAL_PWR_RCC_OSCILLATOR_TYPE;
    RCC_OscInitStruct.HSEState      = HAL_PWR_RCC_HSE_STATE;
    RCC_OscInitStruct.PLL.PLLState  = HAL_PWR_RCC_PLL_STATE;
    RCC_OscInitStruct.PLL.PLLSource = HAL_PWR_RCC_PLL_SOURCE;
    RCC_OscInitStruct.PLL.PLLM      = HAL_PWR_RCC_PLL_M;
    RCC_OscInitStruct.PLL.PLLN      = HAL_PWR_RCC_PLL_N;
    RCC_OscInitStruct.PLL.PLLP      = HAL_PWR_RCC_PLL_P;
    RCC_OscInitStruct.PLL.PLLQ      = HAL_PWR_RCC_PLL_Q;
    if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct)) {
        /* Initialization Error */
        HAL_ASSERT(OS_FALSE);
    }
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
    RCC_ClkInitStruct.ClockType     = HAL_PWR_RCC_CLOCK_TYPE;
    RCC_ClkInitStruct.SYSCLKSource  = HAL_PWR_RCC_CLOCK_SYS_SOURCE;
    RCC_ClkInitStruct.AHBCLKDivider = HAL_PWR_RCC_CLOCK_DIV_AHB;
    RCC_ClkInitStruct.APB1CLKDivider= HAL_PWR_RCC_CLOCK_DIV_APB1;
    RCC_ClkInitStruct.APB2CLKDivider= HAL_PWR_RCC_CLOCK_DIV_APB2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, HAL_PWR_FLASH_LATENCY);

    HAL_RCC_EnableCSS();
}

#ifndef NDEBUG
/******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line);
void assert_failed(uint8_t* file, uint32_t line)
{
    HAL_GPIO_ASSERT_UP();
    /* User can add his own implementation to report the file name and line number,*/
    printf("\nASSERT: %s on line %d\r\n", file, line);
    HAL_CRITICAL_SECTION_ENTER();
    /* Infinite loop */
    while (1) {};
}

/******************************************************************************/
void HAL_ASSERT_FAILED(U8* file, U32 line)
{
    assert_failed(file, line);
}
#endif // NDEBUG