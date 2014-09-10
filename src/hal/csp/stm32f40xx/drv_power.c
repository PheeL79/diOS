/**************************************************************************//**
* @file    drv_power.c
* @brief   Power driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_debug.h"
#include "os_supervise.h"
#include "os_signal.h"
#include "os_message.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_power"

//-----------------------------------------------------------------------------
/// @brief   NVIC initialization.
/// @return  #Status.
Status POWER_Init_(void);

static Status POWER__Init(void* args_p);
static Status POWER__DeInit(void* args_p);
static Status POWER_Open(void* args_p);
static Status POWER_Close(void* args_p);
static Status POWER_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_power_v[DRV_ID_POWER_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_power = {
    .Init   = POWER__Init,
    .DeInit = POWER__DeInit,
    .Open   = POWER_Open,
    .Close  = POWER_Close,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = POWER_IoCtl
};

/*****************************************************************************/
Status POWER_Init_(void)
{
    HAL_MemSet(drv_power_v, 0x0, sizeof(drv_power_v));
    drv_power_v[DRV_ID_POWER] = &drv_power;
    return S_OK;
}

/*****************************************************************************/
Status POWER__Init(void* args_p)
{
    //D_LOG(D_INFO, "Init: ");
    //D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status POWER__DeInit(void* args_p)
{
Status s = S_OK;
//    D_LOG(D_INFO, "DeInit: ");
//    D_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status POWER_Open(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status POWER_Close(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status POWER_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
const OS_PowerState state = *(OS_PowerState*)args_p;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (state) {
                case PWR_STARTUP:
                    break;
                case PWR_OFF:
                    break;
                case PWR_ON:
                    break;
                case PWR_SLEEP:
                    /* Suspend Tick increment to prevent wakeup by Systick interrupt.
                     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
                    HAL_SYSTICK_STOP();
                    /* Request to enter SLEEP mode */
                    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
                    /* Resume Tick interrupt if disabled prior to sleep mode entry */
                    HAL_SYSTICK_START();
                    break;
                case PWR_STOP:
                    OS_CriticalSectionEnter(); {
                        OS_SchedulerSuspend();
                        HAL_SYSTICK_STOP();
                        /* FLASH Deep Power Down Mode enabled */
                        HAL_PWREx_EnableFlashPowerDown();
                        __SEV(); /* Set event to bring the event register bit into a known state (1) */
                        __WFE(); /* This will clear the event register and immediately continue to the next instruction and make you ready to go to sleep using WFE */
                        /* Enter Stop Mode */
                        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
                        /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
                        PLL as system clock source (HSE and PLL are disabled in STOP mode) */
                        RCC_ClkInitTypeDef RCC_ClkInitStruct;
                        RCC_OscInitTypeDef RCC_OscInitStruct;
                        uint32_t pFLatency = 0;
                        /* Get the Oscillators configuration according to the internal RCC registers */
                        HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
                        /* After wake-up from STOP reconfigure the system clock: Enable HSE and PLL */
                        RCC_OscInitStruct.OscillatorType= RCC_OSCILLATORTYPE_HSE;
                        RCC_OscInitStruct.HSEState      = RCC_HSE_ON;
                        RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
                        if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct)) {
                            HAL_ASSERT(OS_FALSE);
                        }
                        /* Get the Clocks configuration according to the internal RCC registers */
                        HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
                        /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
                         clocks dividers */
                        RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_SYSCLK;
                        RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
                        if (HAL_OK != HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency)) {
                            HAL_ASSERT(OS_FALSE);
                        }
//                        HAL_PWREx_DisableFlashPowerDown();
                        HAL_SYSTICK_START();
                        OS_SchedulerResume();
                    } OS_CriticalSectionExit();
                    break;
                case PWR_STANDBY:
                    break;
                case PWR_HIBERNATE:
                    break;
                case PWR_SHUTDOWN:
                    break;
                case PWR_BATTERY:
                    break;
                default:
                    OS_LOG_S(D_WARNING, s = S_INVALID_STATE);
                    break;
            }
            break;
        default:
            break;
    }
    return s;
}
