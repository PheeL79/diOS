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

static Status POWER__Init(void);
static Status POWER__DeInit(void);
static Status POWER_Open(void* args_p);
static Status POWER_Close(void);
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
    memset(drv_power_v, 0x0, sizeof(drv_power_v));
    drv_power_v[DRV_ID_POWER] = &drv_power;
    return S_OK;
}

/*****************************************************************************/
Status POWER__Init(void)
{
    //D_LOG(D_INFO, "Init: ");
    //D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status POWER__DeInit(void)
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
Status POWER_Close(void)
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
        case DRV_REQ_STD_POWER:
            switch (state) {
                case PWR_STARTUP:
                    break;
                case PWR_OFF:
                    break;
                case PWR_ON:
                    break;
                case PWR_SLEEP:
                    __WFI();
                    break;
                case PWR_STOP:
                    OS_SchedulerSuspend();
                    SYSTICK_STOP();
                    OS_CriticalSectionEnter(); {
                        __SEV(); /* Set event to bring the event register bit into a known state (1) */
                        __WFE(); /* This will clear the event register and immediately continue to the next instruction and make you ready to go to sleep using WFE */
                        /* Enter Stop Mode */
                        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
                        /* After wake-up from STOP reconfigure the system clock */
                        /* Enable HSE */
                        RCC_HSEConfig(RCC_HSE_ON);
                        /* Wait till HSE is ready */
                        while (RESET == RCC_GetFlagStatus(RCC_FLAG_HSERDY)) {};
                        /* Enable PLL */
                        RCC_PLLCmd(ENABLE);
                        /* Wait till PLL is ready */
                        while (RESET == RCC_GetFlagStatus(RCC_FLAG_PLLRDY)) {};
                        /* Select PLL as system clock source */
                        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
                        /* Wait till PLL is used as system clock source */
                        while (0x08 != RCC_GetSYSCLKSource()) {};
                    } OS_CriticalSectionExit();
                    SYSTICK_START();
                    OS_SchedulerResume();
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
