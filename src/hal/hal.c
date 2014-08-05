/**************************************************************************//**
* @file    hal_olimex_stm32_p407.c
* @brief   BSP Hardware Abstraction Layer.
* @author  A. Filyanov
******************************************************************************/
#include <yfuns.h>
#include <string.h>
#include "hal.h"
#include "hal_config.h"
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

extern Status   DMA_Init_(void);
extern Status   RTC_Init_(void);
extern Status   NVIC_Init_(void);
extern Status   GPIO_Init_(void);
extern Status   SDIO_Init_(void);
extern Status   TIMER_Init_(void);
extern Status   USART_Init_(void);
extern Status   POWER_Init_(void);

extern Status   MEM_EXT_Init_(void);
extern Status   BUTTON_Init_(void);
extern Status   LED_Init_(void);

/*****************************************************************************/
Status HAL_Init_(void)
{
const Version* ver_p = &device_state.description.device_description.version;
Status s = S_OK;
    // Disable interrupts.
    CRITICAL_ENTER();
    SystemInit();
    if (HAL_OK != HAL_Init()) { return S_HARDWARE_FAULT; }
    SystemClock_Config();
    SystemCoreClockKHz  = SystemCoreClock / KHZ;
    SystemCoreClockMHz  = SystemCoreClockKHz / KHZ;
    TIMER_DWT_Init(); // HAL_LOG depends on this init.
    // HAL environment init
    hal_env.locale      = LOC_EN;
    hal_env.power       = PWR_ON;
    hal_env.log_level   = HAL_LOG_LEVEL_DEFAULT;
    IF_STATUS(s = USART_Init_()) { return s; }
    hal_env.stdio_p = drv_stdio_p;
    HAL_ASSERT(S_OK == hal_env.stdio_p->Init());
    HAL_ASSERT(S_OK == hal_env.stdio_p->Open(OS_NULL));
    // Init device description.
    IF_STATUS(HAL_DeviceDescriptionInit()) { return S_MODULE; }
    // Init and open STDIO stream.
    HAL_StdIoCls();
    HAL_LOG(D_INFO, "-------------------------------");
    HAL_LOG(D_INFO, "diOS: v%d.%d.%d%s-%s",
                   ver_p->maj,
                   ver_p->min,
                   ver_p->bld,
                   ver_lbl[ver_p->lbl],
                   ver_p->rev);
    HAL_LOG(D_INFO, "Built on: %s, %s", __DATE__, __TIME__);
    HAL_LOG(D_INFO, "-------------------------------");
    HAL_LOG(D_INFO, "HAL init...");
    HAL_LOG(D_INFO, "-------------------------------");

    //TODO(A. Filyanov) HAL_CSP_Init(); HAL_MSP_Init()?; HAL_BSP_Init();

    // Low-level drivers init.
    IF_STATUS(s = GPIO_Init_()) { return s; }
    IF_STATUS(s = drv_gpio_v[DRV_ID_GPIOC]->Init()) { return s; }
    IF_STATUS(s = drv_gpio_v[DRV_ID_GPIOF]->Init()) { return s; }
    IF_STATUS(s = NVIC_Init_()) { return s; }
    IF_STATUS(s = drv_nvic_v[DRV_ID_NVIC]->Init()) { return s; }
    IF_STATUS(s = DMA_Init_()) { return s; }
    IF_STATUS(s = drv_dma_v[DRV_ID_DMA2]->Init()) { return s; }
    IF_STATUS(s = RTC_Init_()) { return s; }
    //IF_STATUS(s = drv_rtc[DRV_ID_RTC]->Init()) { return s; }
    IF_STATUS(s = POWER_Init_()) { return s; }
#ifdef DATA_IN_ExtSRAM
    IF_STATUS(s = MEM_EXT_Init_()) { return s; }
    IF_STATUS(s = drv_mem_ext_v[DRV_ID_MEM_EXT_SRAM512K]->Init()) { return s; }
#endif // DATA_IN_ExtSRAM
    IF_STATUS(s = TIMER_Init_()) { return s; }
    IF_STATUS(s = drv_timer_v[DRV_ID_TIMER5]->Init()) { return s; }
    IF_STATUS(s = drv_timer_v[DRV_ID_TIMER10]->Init()) { return s; }
//#ifdef FILE_SYSTEM_ENABLED
    IF_STATUS(s = SDIO_Init_()) { return s; }
//#endif // FILE_SYSTEM_ENABLED
    // High-level drivers init.
    IF_STATUS(s = LED_Init_()) { return s; }
//    IF_STATUS(s = drv_led[DRV_ID_LED_PULSE]->Init()) { return s; }
//    IF_STATUS(s = drv_led[DRV_ID_LED_FS]->Init()) { return s; }
    IF_STATUS(s = drv_led_v[DRV_ID_LED_ASSERT]->Init()) { return s; }
    IF_STATUS(s = drv_led_v[DRV_ID_LED_USER]->Init()) { return s; }
    IF_STATUS(s = BUTTON_Init_()) { return s; }
    IF_STATUS(s = drv_button_v[DRV_ID_BUTTON_WAKEUP]->Init()) { return s; }
    IF_STATUS(s = drv_button_v[DRV_ID_BUTTON_TAMPER]->Init()) { return s; }
    // Enable interrupts.
    HAL_LOG(D_INFO, "-------------------------------");
    // Close and deinit STDIO stream(for init HAL output).
    // Stream will be open back again in OSAL init.
    HAL_ASSERT(S_OK == hal_env.stdio_p->Close());
    HAL_ASSERT(S_OK == hal_env.stdio_p->DeInit());
    CRITICAL_EXIT();
    return s;
}

/*****************************************************************************/
Status HAL_DeviceDescriptionInit(void)
{
    memset((void*)&device_state, 0x0, sizeof(device_state));
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
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();
    /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;                        //TODO(A. Filyanov) Move values to the hal_config.h!
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        /* Initialization Error */
        HAL_ASSERT(OS_FALSE);
    }
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/******************************************************************************/
#ifndef NDEBUG
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,*/
    printf("ASSERT: file %s on line %d\r\n", file, line);
    /* Infinite loop */
    while (1) {};
}
#endif // USE_FULL_ASSERT
#endif // NDEBUG
